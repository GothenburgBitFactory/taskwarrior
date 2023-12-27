use super::service::{ObjectInfo, Service};
use crate::errors::{Error, Result};
use crate::server::encryption::{Cryptor, Sealed, Unsealed};
use crate::server::{
    AddVersionResult, GetVersionResult, HistorySegment, Server, Snapshot, SnapshotUrgency,
    VersionId,
};
use ring::rand;
use std::collections::{HashMap, HashSet};
#[cfg(not(test))]
use std::time::{SystemTime, UNIX_EPOCH};
use uuid::Uuid;

/// Implement the Server trait for a cloud service implemented by [`Service`].
///
/// This type implements a TaskChampion server over a basic object-storage service. It encapsulates
/// all of the logic to ensure a linear sequence of versions, encrypt and decrypt data, and clean
/// up old data so that this can be supported on a variety of cloud services.
///
/// ## Encryption
///
/// The encryption scheme is described in `sync-protocol.md`. The salt value used for key
/// derivation is stored in "salt", which is created if it does not exist. Object names are not
/// encrypted, by the nature of key/value stores. Since the content of the "latest" object can
/// usually be inferred from object names, it, too, is not encrypted.
///
/// ## Object Organization
///
/// UUIDs emebedded in names and values appear in their "simple" form: lower-case hexadecimal with
/// no hyphens.
///
/// Versions are stored as objects with name `v-PARENT-VERSION` where `PARENT` is the parent
/// version's UUID and `VERSION` is the version's UUID. The object value is the raw history
/// segment. These objects are created with simple `put` requests, as the name uniquely identifies
/// the content.
///
/// The latest version is stored as an object with name "latest", containing the UUID of the latest
/// version. This file is updated with `compare_and_swap`. After a successful update of this
/// object, the version is considered committed.
///
/// Since there are no strong constraints on creation of version objects, it is possible
/// to have multiple such files with the same `PARENT`. However, only one such object will be
/// contained in the chain of parent-child relationships beginning with the value in "latest".
/// All other objects are invalid and not visible outside this type.
///
/// Snapshots are stored as objects with name `s-VERSION` where `VERSION` is the version at which
/// the snapshot was made. These objects are created with simple `put` requests, as any snapshot
/// for a given version is functionally equivalent to any other.
///
/// ## Cleanup
///
/// Cleanup of unnecessary data is performed probabalistically after `add_version`, although any
/// errors are ignored.
///
///  - Any versions not reachable from "latest" and which cannot become "latest" are deleted.
///  - Any snapshots older than the most recent are deleted.
///  - Any versions older than [`MAX_VERSION_AGE_SECS`] which are incorporated into a snapshot
///    are deleted.
pub(in crate::server) struct CloudServer<SVC: Service> {
    service: SVC,

    /// The Cryptor supporting encryption and decryption of objects in this server.
    cryptor: Cryptor,

    /// The probability (0..255) that this run will perform cleanup.
    cleanup_probability: u8,

    /// The last version added locally with the `add_version` method.
    last_version_added: Option<VersionId>,

    /// For testing, a function that is called in the middle of `add_version` to simulate
    /// a concurrent change in the service.
    #[cfg(test)]
    add_version_intercept: Option<fn(service: &mut SVC)>,
}

const LATEST: &[u8] = b"latest";
const DEFAULT_CLEANUP_PROBABILITY: u8 = 13; // about 5%

#[cfg(not(test))]
const MAX_VERSION_AGE_SECS: u64 = 3600 * 24 * 180; // about half a year

fn version_to_bytes(v: VersionId) -> Vec<u8> {
    v.as_simple().to_string().into_bytes()
}

impl<SVC: Service> CloudServer<SVC> {
    pub(in crate::server) fn new(mut service: SVC, encryption_secret: Vec<u8>) -> Result<Self> {
        let salt = Self::get_salt(&mut service)?;
        let cryptor = Cryptor::new(salt, &encryption_secret.into())?;
        Ok(Self {
            service,
            cryptor,
            cleanup_probability: DEFAULT_CLEANUP_PROBABILITY,
            last_version_added: None,
            #[cfg(test)]
            add_version_intercept: None,
        })
    }

    /// Get the salt value stored in the service, creating a new random one if necessary.
    fn get_salt(service: &mut SVC) -> Result<Vec<u8>> {
        const SALT_NAME: &[u8] = b"salt";
        loop {
            if let Some(salt) = service.get(SALT_NAME)? {
                return Ok(salt);
            }
            service.compare_and_swap(SALT_NAME, None, Cryptor::gen_salt()?)?;
        }
    }

    /// Generate an object name for the given parent and child versions.
    fn version_name(parent_version_id: &VersionId, child_version_id: &VersionId) -> Vec<u8> {
        format!(
            "v-{}-{}",
            parent_version_id.as_simple(),
            child_version_id.as_simple()
        )
        .into_bytes()
    }

    /// Parse a version name as generated by `version_name`, returning None if the name does not
    /// have a valid format.
    fn parse_version_name(name: &[u8]) -> Option<(VersionId, VersionId)> {
        if name.len() != 2 + 32 + 1 + 32 || !name.starts_with(b"v-") || name[2 + 32] != b'-' {
            return None;
        }
        let Ok(parent_version_id) = VersionId::try_parse_ascii(&name[2..2 + 32]) else {
            return None;
        };
        let Ok(child_version_id) = VersionId::try_parse_ascii(&name[2 + 32 + 1..]) else {
            return None;
        };
        Some((parent_version_id, child_version_id))
    }

    /// Generate an object name for a snapshot at the given version.
    fn snapshot_name(version_id: &VersionId) -> Vec<u8> {
        format!("s-{}", version_id.as_simple()).into_bytes()
    }

    /// Parse a snapshot name as generated by `snapshot_name`, returning None if the name does not
    /// have a valid format.
    fn parse_snapshot_name(name: &[u8]) -> Option<VersionId> {
        if name.len() != 2 + 32 || !name.starts_with(b"s-") {
            return None;
        }
        let Ok(version_id) = VersionId::try_parse_ascii(&name[2..2 + 32]) else {
            return None;
        };
        Some(version_id)
    }

    /// Generate a random integer in (0..255) for use in probabalistic decisions.
    fn randint(&self) -> Result<u8> {
        use rand::SecureRandom;
        let mut randint = [0u8];
        rand::SystemRandom::new()
            .fill(&mut randint)
            .map_err(|_| Error::Server("Random number generator failure".into()))?;
        Ok(randint[0])
    }

    /// Get the version from "latest", or None if the object does not exist. This always fetches a fresh
    /// value from storage.
    fn get_latest(&mut self) -> Result<Option<VersionId>> {
        let Some(latest) = self.service.get(LATEST)? else {
            return Ok(None);
        };
        let latest = VersionId::try_parse_ascii(&latest)
            .map_err(|_| Error::Server("'latest' object contains invalid data".into()))?;
        Ok(Some(latest))
    }

    /// Get the possible child versions of the given parent version, based only on the object
    /// names.
    fn get_child_versions(&mut self, parent_version_id: &VersionId) -> Result<Vec<VersionId>> {
        self.service
            .list(format!("v-{}-", parent_version_id.as_simple()).as_bytes())
            .filter_map(|res| match res {
                Ok(ObjectInfo { name, .. }) => {
                    if let Some((_, c)) = Self::parse_version_name(&name) {
                        Some(Ok(c))
                    } else {
                        None
                    }
                }
                Err(e) => Some(Err(e)),
            })
            .collect::<Result<Vec<_>>>()
    }

    /// Determine the snapshot urgency. This is done probabalistically:
    ///  - High urgency approximately 1% of the time.
    ///  - Low urgency approximately 10% of the time.
    fn snapshot_urgency(&self) -> Result<SnapshotUrgency> {
        let r = self.randint()?;
        if r < 2 {
            Ok(SnapshotUrgency::High)
        } else if r < 25 {
            Ok(SnapshotUrgency::Low)
        } else {
            Ok(SnapshotUrgency::None)
        }
    }

    /// Maybe call `cleanup` depending on `cleanup_probability`.
    fn maybe_cleanup(&mut self) -> Result<()> {
        if self.randint()? < self.cleanup_probability {
            self.cleanup_probability = DEFAULT_CLEANUP_PROBABILITY;
            self.cleanup()
        } else {
            Ok(())
        }
    }

    /// Perform cleanup, deleting unnecessary data.
    fn cleanup(&mut self) -> Result<()> {
        // Construct a vector containing all (child, parent, creation) tuples
        let mut versions = self
            .service
            .list(b"v-")
            .filter_map(|res| match res {
                Ok(ObjectInfo { name, creation }) => {
                    if let Some((p, c)) = Self::parse_version_name(&name) {
                        Some(Ok((c, p, creation)))
                    } else {
                        None
                    }
                }
                Err(e) => Some(Err(e)),
            })
            .collect::<Result<Vec<_>>>()?;
        versions.sort();

        // Function to find the parent of a given child version in `versions`, taking
        // advantage of having sorted the vector by child version ID.
        let parent_of = |c| match versions.binary_search_by_key(&c, |tup| tup.0) {
            Ok(idx) => Some(versions[idx].1),
            Err(_) => None,
        };

        // Create chains mapping forward (parent -> child) and backward (child -> parent), starting
        // at "latest".
        let mut rev_chain = HashMap::new();
        let mut iterations = versions.len() + 1; // For cycle detection.
        let latest = self.get_latest()?;
        if let Some(mut c) = latest {
            while let Some(p) = parent_of(c) {
                rev_chain.insert(c, p);
                c = p;
                iterations -= 1;
                if iterations == 0 {
                    return Err(Error::Server("Version cycle detected".into()));
                }
            }
        }

        // Collect all versions older than MAX_VERSION_AGE_SECS
        #[cfg(not(test))]
        let age_threshold = {
            let now = SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .map(|t| t.as_secs())
                .unwrap_or(0);
            now.saturating_sub(MAX_VERSION_AGE_SECS)
        };

        // In testing, cutoff age is 1000.
        #[cfg(test)]
        let age_threshold = 1000;

        let old_versions: HashSet<Uuid> = versions
            .iter()
            .filter_map(|(c, _, creation)| {
                if *creation < age_threshold {
                    Some(*c)
                } else {
                    None
                }
            })
            .collect();

        // Now, any pair not present in that chain can be deleted. However, another replica
        // may be in the state where it has uploaded a version but not changed "latest" yet,
        // so any pair with parent equal to latest is allowed to stay.
        for (c, p, _) in versions {
            if rev_chain.get(&c) != Some(&p) && Some(p) != latest {
                self.service.del(&Self::version_name(&p, &c))?;
            }
        }

        // Collect a set of all snapshots.
        let snapshots = self
            .service
            .list(b"s-")
            .filter_map(|res| match res {
                Ok(ObjectInfo { name, .. }) => Self::parse_snapshot_name(&name).map(Ok),
                Err(e) => Some(Err(e)),
            })
            .collect::<Result<HashSet<_>>>()?;

        // Find the latest snapshot by iterating back from "latest". Note that this iteration is
        // guaranteed not to be cyclical, as that was checked above.
        let mut latest_snapshot = None;
        if let Some(mut version) = latest {
            loop {
                if snapshots.contains(&version) {
                    latest_snapshot = Some(version);
                    break;
                }
                if let Some(v) = rev_chain.get(&version) {
                    version = *v;
                } else {
                    break;
                }
            }
        }

        // If there's a latest snapshot, delete all other snapshots.
        let Some(latest_snapshot) = latest_snapshot else {
            // If there's no snapshot, no further cleanup is possible.
            return Ok(());
        };
        for version in snapshots {
            if version != latest_snapshot {
                self.service.del(&Self::snapshot_name(&version))?;
            }
        }

        // Now continue iterating backward from that version; any version in `old_versions` can be
        // deleted.
        let mut version = latest_snapshot;
        while let Some(parent) = rev_chain.get(&version) {
            if old_versions.contains(&version) {
                self.service.del(&Self::version_name(parent, &version))?;
            }
            version = *parent;
        }

        Ok(())
    }
}

impl<SVC: Service> Server for CloudServer<SVC> {
    fn add_version(
        &mut self,
        parent_version_id: VersionId,
        history_segment: HistorySegment,
    ) -> Result<(AddVersionResult, SnapshotUrgency)> {
        let latest = self.get_latest()?;
        if let Some(l) = latest {
            if l != parent_version_id {
                return Ok((
                    AddVersionResult::ExpectedParentVersion(l),
                    self.snapshot_urgency()?,
                ));
            }
        }

        // Invent a new version ID and upload the version data.
        let version_id = VersionId::new_v4();
        let new_name = Self::version_name(&parent_version_id, &version_id);
        let sealed = self.cryptor.seal(Unsealed {
            version_id,
            payload: history_segment,
        })?;
        self.service.put(&new_name, sealed.as_ref())?;

        #[cfg(test)]
        if let Some(f) = self.add_version_intercept {
            f(&mut self.service);
        }

        // Try to compare-and-swap this value into LATEST
        let old_value = latest.map(version_to_bytes);
        let new_value = version_to_bytes(version_id);
        if !self
            .service
            .compare_and_swap(LATEST, old_value, new_value)?
        {
            // Delete the version data, since it was not latest.
            self.service.del(&new_name)?;
            let latest = self.get_latest()?;
            let latest = latest.unwrap_or(Uuid::nil());
            return Ok((
                AddVersionResult::ExpectedParentVersion(latest),
                self.snapshot_urgency()?,
            ));
        }

        // Attempt a cleanup, but ignore errors.
        let _ = self.maybe_cleanup();

        // Record the newly returned `version_id`, to verify that this is the snapshot we may get.
        self.last_version_added = Some(version_id);

        Ok((AddVersionResult::Ok(version_id), self.snapshot_urgency()?))
    }

    fn get_child_version(&mut self, parent_version_id: VersionId) -> Result<GetVersionResult> {
        // The `get_child_versions` function will usually return only one child version for a
        // parent, in which case the work is easy. Otherwise, if there are several possible
        // children, only one of those will lead to `latest`, and importantly the others will not
        // have their own children. So we can detect the "true" child as the one that is equal to
        // "latest" or has children.
        let version_id = match &(self.get_child_versions(&parent_version_id)?)[..] {
            [] => return Ok(GetVersionResult::NoSuchVersion),
            [child] => *child,
            children => {
                // There are some extra version objects, so a cleanup is warranted.
                self.cleanup_probability = 255;
                let latest = self.get_latest()?;
                let mut true_child = None;
                for child in children {
                    if Some(*child) == latest {
                        true_child = Some(*child);
                        break;
                    }
                }
                if true_child.is_none() {
                    for child in children {
                        if !self.get_child_versions(child)?.is_empty() {
                            true_child = Some(*child)
                        }
                    }
                }
                match true_child {
                    Some(true_child) => true_child,
                    None => return Ok(GetVersionResult::NoSuchVersion),
                }
            }
        };

        let Some(sealed) = self
            .service
            .get(&Self::version_name(&parent_version_id, &version_id))?
        else {
            // This really shouldn't happen, since the chain was derived from object names, but
            // perhaps the object was deleted.
            return Ok(GetVersionResult::NoSuchVersion);
        };
        let unsealed = self.cryptor.unseal(Sealed {
            version_id,
            payload: sealed,
        })?;
        Ok(GetVersionResult::Version {
            version_id,
            parent_version_id,
            history_segment: unsealed.into(),
        })
    }

    fn add_snapshot(&mut self, version_id: VersionId, snapshot: Snapshot) -> Result<()> {
        // The protocol says to verify that this is more recent than any other snapshot, and
        // corresponds to an existing version. In practice, this will always be the version
        // just returned from add_version, so just check that.
        if Some(version_id) != self.last_version_added {
            return Err(Error::Server("Snapshot is not for most recently added version".into()));
        }
        let name = Self::snapshot_name(&version_id);
        let sealed = self.cryptor.seal(Unsealed {
            version_id,
            payload: snapshot,
        })?;
        self.service.put(&name, sealed.as_ref())?;
        Ok(())
    }

    fn get_snapshot(&mut self) -> Result<Option<(VersionId, Snapshot)>> {
        // Pick the first snapshot we find.
        let Some(name) = self.service.list(b"s-").next() else {
            return Ok(None);
        };
        let ObjectInfo { name, .. } = name?;
        let Some(version_id) = Self::parse_snapshot_name(&name) else {
            return Ok(None);
        };
        let Some(payload) = self.service.get(&name)? else {
            return Ok(None);
        };
        let unsealed = self.cryptor.unseal(Sealed {
            version_id,
            payload,
        })?;
        Ok(Some((version_id, unsealed.payload)))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::server::NIL_VERSION_ID;

    /// A simple in-memory service for testing. All insertions via Service methods occur at time
    /// `INSERTION_TIME`. All versions older that 1000 are considered "old".
    #[derive(Clone)]
    struct MockService(HashMap<Vec<u8>, (u64, Vec<u8>)>);

    const INSERTION_TIME: u64 = 9999999999;

    impl MockService {
        fn new() -> Self {
            let mut map = HashMap::new();
            // Use a fixed salt for consistent results
            map.insert(b"salt".to_vec(), (0, b"abcdefghabcdefgh".to_vec()));
            Self(map)
        }
    }

    impl Service for MockService {
        fn put(&mut self, name: &[u8], value: &[u8]) -> Result<()> {
            self.0
                .insert(name.to_vec(), (INSERTION_TIME, value.to_vec()));
            Ok(())
        }

        fn get(&mut self, name: &[u8]) -> Result<Option<Vec<u8>>> {
            Ok(self.0.get(name).map(|(_, data)| data.clone()))
        }

        fn del(&mut self, name: &[u8]) -> Result<()> {
            self.0.remove(name);
            Ok(())
        }

        fn compare_and_swap(
            &mut self,
            name: &[u8],
            existing_value: Option<Vec<u8>>,
            new_value: Vec<u8>,
        ) -> Result<bool> {
            if self.0.get(name).map(|(_, d)| d) == existing_value.as_ref() {
                self.0.insert(name.to_vec(), (INSERTION_TIME, new_value));
                return Ok(true);
            }
            Ok(false)
        }

        fn list<'a>(
            &'a mut self,
            prefix: &[u8],
        ) -> Box<dyn Iterator<Item = Result<ObjectInfo>> + 'a> {
            let prefix = prefix.to_vec();
            Box::new(
                self.0
                    .iter()
                    .filter(move |(k, _)| k.starts_with(&prefix))
                    .map(|(k, (t, _))| {
                        Ok(ObjectInfo {
                            name: k.to_vec(),
                            creation: *t,
                        })
                    }),
            )
        }
    }

    impl std::fmt::Debug for MockService {
        fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
            f.debug_map()
                .entries(
                    self.0
                        .iter()
                        .map(|(k, v)| (std::str::from_utf8(k).unwrap(), v)),
                )
                .finish()
        }
    }

    // Add some testing utilities to CloudServer.
    impl CloudServer<MockService> {
        fn mock_add_version(
            &mut self,
            parent: VersionId,
            child: VersionId,
            creation: u64,
            data: &[u8],
        ) {
            let name = Self::version_name(&parent, &child);
            let sealed = self
                .cryptor
                .seal(Unsealed {
                    version_id: child,
                    payload: data.into(),
                })
                .unwrap();
            self.service.0.insert(name, (creation, sealed.into()));
        }

        fn mock_add_snapshot(&mut self, version: VersionId, creation: u64, snapshot: &[u8]) {
            let name = Self::snapshot_name(&version);
            let sealed = self
                .cryptor
                .seal(Unsealed {
                    version_id: version,
                    payload: snapshot.into(),
                })
                .unwrap();
            self.service.0.insert(name, (creation, sealed.into()));
        }

        fn mock_set_latest(&mut self, latest: VersionId) {
            let latest = version_to_bytes(latest);
            self.service
                .0
                .insert(LATEST.to_vec(), (INSERTION_TIME, latest));
        }

        /// Create a copy of this server without any data; used for creating a MockService
        /// to compare to with `assert_eq!`
        fn empty_clone(&self) -> Self {
            Self {
                cryptor: self.cryptor.clone(),
                cleanup_probability: 0,
                last_version_added: None,
                service: MockService::new(),
                add_version_intercept: None,
            }
        }

        /// Get a decrypted, string-y copy of the data in the HashMap.
        fn unencrypted(&self) -> HashMap<String, (u64, String)> {
            self.service
                .0
                .iter()
                .map(|(k, v)| {
                    let kstr = String::from_utf8(k.to_vec()).unwrap();
                    if kstr == "latest" {
                        return (kstr, (v.0, String::from_utf8(v.1.to_vec()).unwrap()));
                    }

                    let version_id;
                    if let Some((_, v)) = Self::parse_version_name(k) {
                        version_id = v;
                    } else if let Some(v) = Self::parse_snapshot_name(k) {
                        version_id = v;
                    } else {
                        return (kstr, (v.0, format!("{:?}", v.1)));
                    }

                    let unsealed = self
                        .cryptor
                        .unseal(Sealed {
                            version_id,
                            payload: v.1.to_vec(),
                        })
                        .unwrap();
                    let vstr = String::from_utf8(unsealed.into()).unwrap();
                    (kstr, (v.0, vstr))
                })
                .collect()
        }
    }
    impl Clone for CloudServer<MockService> {
        fn clone(&self) -> Self {
            Self {
                cryptor: self.cryptor.clone(),
                cleanup_probability: self.cleanup_probability,
                last_version_added: self.last_version_added,
                service: self.service.clone(),
                add_version_intercept: None,
            }
        }
    }

    const SECRET: &[u8] = b"testing";

    fn make_server() -> CloudServer<MockService> {
        let mut server = CloudServer::new(MockService::new(), SECRET.into()).unwrap();
        // Prevent cleanup during tests.
        server.cleanup_probability = 0;
        server
    }

    #[test]
    fn version_name() {
        let p = Uuid::parse_str("a1a2a3a4b1b2c1c2d1d2d3d4d5d6d7d8").unwrap();
        let c = Uuid::parse_str("adcf4e350fa54e4aaf9d3f20f3ba5a32").unwrap();
        assert_eq!(
            CloudServer::<MockService>::version_name(&p, &c),
            b"v-a1a2a3a4b1b2c1c2d1d2d3d4d5d6d7d8-adcf4e350fa54e4aaf9d3f20f3ba5a32"
        );
    }

    #[test]
    fn version_name_round_trip() {
        let p = Uuid::new_v4();
        let c = Uuid::new_v4();
        assert_eq!(
            CloudServer::<MockService>::parse_version_name(
                &CloudServer::<MockService>::version_name(&p, &c)
            ),
            Some((p, c))
        );
    }

    #[test]
    fn parse_version_name_bad_prefix() {
        assert_eq!(
            CloudServer::<MockService>::parse_version_name(
                b"X-a1a2a3a4b1b2c1c2d1d2d3d4d5d6d7d8-adcf4e350fa54e4aaf9d3f20f3ba5a32"
            ),
            None
        );
    }

    #[test]
    fn parse_version_name_bad_separator() {
        assert_eq!(
            CloudServer::<MockService>::parse_version_name(
                b"v-a1a2a3a4b1b2c1c2d1d2d3d4d5d6d7d8xadcf4e350fa54e4aaf9d3f20f3ba5a32"
            ),
            None
        );
    }

    #[test]
    fn parse_version_name_too_short() {
        assert_eq!(
            CloudServer::<MockService>::parse_version_name(
                b"v-a1a2a3a4b1b2c1c2d1d2d3d4d5d6d7d8-adcf4e350fa54e4aaf9d3f20f3ba5a3"
            ),
            None
        );
    }

    #[test]
    fn parse_version_name_too_long() {
        assert_eq!(
            CloudServer::<MockService>::parse_version_name(
                b"v-a1a2a3a4b1b2c1c2d1d2d3d4d5d6d7d8-adcf4e350fa54e4aaf9d3f20f3ba5a320"
            ),
            None
        );
    }

    #[test]
    fn snapshot_name_round_trip() {
        let v = Uuid::new_v4();
        assert_eq!(
            CloudServer::<MockService>::parse_snapshot_name(
                &CloudServer::<MockService>::snapshot_name(&v)
            ),
            Some(v)
        );
    }

    #[test]
    fn parse_snapshot_name_invalid() {
        assert_eq!(
            CloudServer::<MockService>::parse_snapshot_name(b"s-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"),
            None
        );
    }

    #[test]
    fn parse_snapshot_name_bad_prefix() {
        assert_eq!(
            CloudServer::<MockService>::parse_snapshot_name(b"s:a1a2a3a4b1b2c1c2d1d2d3d4d5d6d7d8"),
            None
        );
    }

    #[test]
    fn parse_snapshot_name_too_short() {
        assert_eq!(
            CloudServer::<MockService>::parse_snapshot_name(b"s-a1a2a3a4b1b2c1c2d1d2d3d4d5d6"),
            None
        );
    }

    #[test]
    fn parse_snapshot_name_too_long() {
        assert_eq!(
            CloudServer::<MockService>::parse_snapshot_name(
                b"s-a1a2a3a4b1b2c1c2d1d2d3d4d5d6d7d8000"
            ),
            None
        );
    }

    #[test]
    fn get_salt_existing() {
        let mut service = MockService::new();
        assert_eq!(
            CloudServer::<MockService>::get_salt(&mut service).unwrap(),
            b"abcdefghabcdefgh".to_vec()
        );
    }

    #[test]
    fn get_salt_create() {
        let mut service = MockService::new();
        service.del(b"salt").unwrap();
        let got_salt = CloudServer::<MockService>::get_salt(&mut service).unwrap();
        let salt_obj = service.get(b"salt").unwrap().unwrap();
        assert_eq!(got_salt, salt_obj);
    }

    #[test]
    fn get_latest_empty() {
        let mut server = make_server();
        assert_eq!(server.get_latest().unwrap(), None);
    }

    #[test]
    fn get_latest_exists() {
        let mut server = make_server();
        let latest = Uuid::new_v4();
        server.mock_set_latest(latest);
        assert_eq!(server.get_latest().unwrap(), Some(latest));
    }

    #[test]
    fn get_latest_invalid() {
        let mut server = make_server();
        server
            .service
            .0
            .insert(LATEST.to_vec(), (999, b"not-a-uuid".to_vec()));
        assert!(server.get_latest().is_err());
    }

    #[test]
    fn get_child_versions_empty() {
        let mut server = make_server();
        assert_eq!(server.get_child_versions(&Uuid::new_v4()).unwrap(), vec![]);
    }

    #[test]
    fn get_child_versions_single() {
        let mut server = make_server();
        let (v1, v2) = (Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(v2, v1, 1000, b"first");
        assert_eq!(server.get_child_versions(&v1).unwrap(), vec![]);
        assert_eq!(server.get_child_versions(&v2).unwrap(), vec![v1]);
    }

    #[test]
    fn get_child_versions_multiple() {
        let mut server = make_server();
        let (v1, v2, v3) = (Uuid::new_v4(), Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(v3, v1, 1000, b"first");
        server.mock_add_version(v3, v2, 1000, b"second");
        assert_eq!(server.get_child_versions(&v1).unwrap(), vec![]);
        assert_eq!(server.get_child_versions(&v2).unwrap(), vec![]);
        let versions = server.get_child_versions(&v3).unwrap();
        assert!(versions == vec![v1, v2] || versions == vec![v2, v1]);
    }

    #[test]
    fn add_version_empty() {
        let mut server = make_server();
        let parent = Uuid::new_v4();
        let (res, _) = server.add_version(parent, b"history".to_vec()).unwrap();
        assert!(matches!(res, AddVersionResult::Ok(_)));
    }

    #[test]
    fn add_version_good() {
        let mut server = make_server();
        let (v1, v2) = (Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(v1, v2, 1000, b"first");
        server.mock_set_latest(v2);

        let (res, _) = server.add_version(v2, b"history".to_vec()).unwrap();
        let AddVersionResult::Ok(new_version) = res else {
            panic!("expected OK");
        };

        let mut expected = server.empty_clone();
        expected.mock_add_version(v1, v2, 1000, b"first");
        expected.mock_add_version(v2, new_version, INSERTION_TIME, b"history");
        expected.mock_set_latest(new_version);

        assert_eq!(server.unencrypted(), expected.unencrypted());
    }

    #[test]
    fn add_version_not_latest() {
        // The `add_version` method does nothing if the version is not latest.
        let mut server = make_server();
        let (v1, v2) = (Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(v1, v2, 1000, b"first");
        server.mock_set_latest(v2);

        let expected = server.clone();

        let (res, _) = server.add_version(v1, b"history".to_vec()).unwrap();
        assert_eq!(res, AddVersionResult::ExpectedParentVersion(v2));
        assert_eq!(server.unencrypted(), expected.unencrypted());
    }

    #[test]
    fn add_version_not_latest_race() {
        // The `add_version` function effectively checks twice for a conflict: once by just
        // fetching "latest", returning early if the value is not as expected; and once in the
        // compare-and-swap. This test uses `add_version_intercept` to force the first check to
        // succeed and the second test to fail.
        let mut server = make_server();
        let (v1, v2) = (Uuid::new_v4(), Uuid::new_v4());
        const V3: Uuid = Uuid::max();
        server.mock_add_version(v1, v2, 1000, b"first");
        server.mock_add_version(v2, V3, 1000, b"second");
        server.mock_set_latest(v2);
        server.add_version_intercept = Some(|service| {
            service.put(LATEST, &version_to_bytes(V3)).unwrap();
        });

        let mut expected = server.empty_clone();
        expected.mock_add_version(v1, v2, 1000, b"first");
        expected.mock_add_version(v2, V3, 1000, b"second");
        expected.mock_set_latest(V3); // updated by the intercept

        assert_ne!(server.unencrypted(), expected.unencrypted());
        let (res, _) = server.add_version(v2, b"history".to_vec()).unwrap();
        assert_eq!(res, AddVersionResult::ExpectedParentVersion(V3));
        assert_eq!(server.unencrypted(), expected.unencrypted());
    }

    #[test]
    fn add_version_unknown() {
        let mut server = make_server();
        let (v1, v2) = (Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(v1, v2, 1000, b"first");
        server.mock_set_latest(v2);

        let expected = server.clone();

        let (res, _) = server
            .add_version(Uuid::new_v4(), b"history".to_vec())
            .unwrap();
        assert_eq!(res, AddVersionResult::ExpectedParentVersion(v2));
        assert_eq!(server.unencrypted(), expected.unencrypted());
    }

    #[test]
    fn get_child_version_empty() {
        let mut server = make_server();
        assert_eq!(
            server.get_child_version(Uuid::new_v4()).unwrap(),
            GetVersionResult::NoSuchVersion
        );
    }

    #[test]
    fn get_child_version_single() {
        let mut server = make_server();
        let (v1, v2) = (Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(v2, v1, 1000, b"first");
        assert_eq!(
            server.get_child_version(v1).unwrap(),
            GetVersionResult::NoSuchVersion
        );
        assert_eq!(
            server.get_child_version(v2).unwrap(),
            GetVersionResult::Version {
                version_id: v1,
                parent_version_id: v2,
                history_segment: b"first".to_vec(),
            }
        );
    }

    #[test]
    fn get_child_version_multiple() {
        let mut server = make_server();
        let (v1, v2, v3) = (Uuid::new_v4(), Uuid::new_v4(), Uuid::new_v4());
        let (vx, vy, vz) = (Uuid::new_v4(), Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(v1, v2, 1000, b"second");
        server.mock_add_version(v1, vx, 1000, b"false start x");
        server.mock_add_version(v1, vy, 1000, b"false start y");
        server.mock_add_version(v2, v3, 1000, b"third");
        server.mock_add_version(v2, vz, 1000, b"false start z");
        server.mock_set_latest(v3);
        assert_eq!(
            server.get_child_version(v1).unwrap(),
            GetVersionResult::Version {
                version_id: v2,
                parent_version_id: v1,
                history_segment: b"second".to_vec(),
            }
        );
        assert_eq!(
            server.get_child_version(v2).unwrap(),
            GetVersionResult::Version {
                version_id: v3,
                parent_version_id: v2,
                history_segment: b"third".to_vec(),
            }
        );
        assert_eq!(
            server.get_child_version(v3).unwrap(),
            GetVersionResult::NoSuchVersion
        );
    }

    #[test]
    fn cleanup_empty() {
        let mut server = make_server();
        server.cleanup().unwrap();
    }

    #[test]
    fn cleanup_linear() {
        // Test that cleanup does nothing for a linear version history with a snapshot at the
        // oldest version.
        let mut server = make_server();
        let (v1, v2, v3) = (Uuid::new_v4(), Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(NIL_VERSION_ID, v1, 1000, b"first");
        server.mock_add_version(v1, v2, 1000, b"second");
        server.mock_add_version(v2, v3, 1000, b"third");
        server.mock_add_snapshot(v1, 1000, b"snap 1");
        server.mock_set_latest(v3);

        let expected = server.clone();

        server.cleanup().unwrap();
        assert_eq!(server.unencrypted(), expected.unencrypted());
    }

    #[test]
    fn cleanup_cycle() {
        // When a cycle is present, cleanup succeeds and makes no changes.
        let mut server = make_server();
        let (v1, v2, v3) = (Uuid::new_v4(), Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(v3, v1, 1000, b"first");
        server.mock_add_version(v1, v2, 1000, b"second");
        server.mock_add_version(v2, v3, 1000, b"third");
        server.mock_set_latest(v3);

        let expected = server.clone();

        assert!(server.cleanup().is_err());
        assert_eq!(server.unencrypted(), expected.unencrypted());
    }

    #[test]
    fn cleanup_extra_branches() {
        // Cleanup deletes extra branches in the versions.
        let mut server = make_server();
        let (v1, v2, v3) = (Uuid::new_v4(), Uuid::new_v4(), Uuid::new_v4());
        let (vx, vy) = (Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(v1, v2, 1000, b"second");
        server.mock_add_version(v1, vx, 1000, b"false start x");
        server.mock_add_version(v2, v3, 1000, b"third");
        server.mock_add_version(v2, vy, 1000, b"false start y");
        server.mock_set_latest(v3);

        let mut expected = server.empty_clone();
        expected.mock_add_version(v1, v2, 1000, b"second");
        expected.mock_add_version(v2, v3, 1000, b"third");
        expected.mock_set_latest(v3);

        assert_ne!(server.unencrypted(), expected.unencrypted());
        server.cleanup().unwrap();
        assert_eq!(server.unencrypted(), expected.unencrypted());
    }

    #[test]
    fn cleanup_extra_snapshots() {
        let mut server = make_server();
        let (v1, v2, v3) = (Uuid::new_v4(), Uuid::new_v4(), Uuid::new_v4());
        let vy = Uuid::new_v4();
        server.mock_add_version(v1, v2, 1000, b"second");
        server.mock_add_version(v2, v3, 1000, b"third");
        server.mock_add_version(v2, vy, 1000, b"false start y");
        server.mock_add_snapshot(v1, 1000, b"snap 1");
        server.mock_add_snapshot(v2, 1000, b"snap 2");
        server.mock_add_snapshot(vy, 1000, b"snap y");
        server.mock_set_latest(v3);

        let mut expected = server.empty_clone();
        expected.mock_add_version(v1, v2, 1000, b"second");
        expected.mock_add_version(v2, v3, 1000, b"third");
        expected.mock_add_snapshot(v2, 1000, b"snap 2");
        expected.mock_set_latest(v3);

        assert_ne!(server.unencrypted(), expected.unencrypted());
        server.cleanup().unwrap();
        assert_eq!(server.unencrypted(), expected.unencrypted());
    }

    #[test]
    fn cleanup_old_versions_no_snapshot() {
        // If there are old versions ,but no snapshot, nothing is cleaned up.
        let mut server = make_server();
        let (v1, v2, v3) = (Uuid::new_v4(), Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(v1, v2, 200, b"second");
        server.mock_add_version(v2, v3, 300, b"third");
        server.mock_set_latest(v3);

        let expected = server.clone();

        server.cleanup().unwrap();
        assert_eq!(server.unencrypted(), expected.unencrypted());
    }

    #[test]
    fn cleanup_old_versions_with_snapshot() {
        // If there are old versions that are also older than a snapshot, they are
        // cleaned up.
        let mut server = make_server();
        let (v1, v2, v3) = (Uuid::new_v4(), Uuid::new_v4(), Uuid::new_v4());
        let (v4, v5, v6) = (Uuid::new_v4(), Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(v1, v2, 200, b"second");
        server.mock_add_version(v2, v3, 300, b"third");
        server.mock_add_version(v3, v4, 1400, b"fourth");
        server.mock_add_version(v4, v5, 1500, b"fifth");
        server.mock_add_snapshot(v5, 1501, b"snap 1");
        server.mock_add_version(v5, v6, 1600, b"sixth");
        server.mock_set_latest(v6);

        let mut expected = server.empty_clone();
        expected.mock_add_version(v3, v4, 1400, b"fourth"); // Not old enough to be deleted.
        expected.mock_add_version(v4, v5, 1500, b"fifth");
        expected.mock_add_snapshot(v5, 1501, b"snap 1");
        expected.mock_add_version(v5, v6, 1600, b"sixth");
        expected.mock_set_latest(v6);

        assert_ne!(server.unencrypted(), expected.unencrypted());
        server.cleanup().unwrap();
        assert_eq!(server.unencrypted(), expected.unencrypted());
    }

    #[test]
    fn cleanup_old_versions_newer_than_snapshot() {
        // Old versions that are newer than the latest snapshot are not cleaned up.
        let mut server = make_server();
        let (v1, v2, v3) = (Uuid::new_v4(), Uuid::new_v4(), Uuid::new_v4());
        let (v4, v5, v6) = (Uuid::new_v4(), Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(v1, v2, 200, b"second");
        server.mock_add_version(v2, v3, 300, b"third");
        server.mock_add_snapshot(v3, 301, b"snap 1");
        server.mock_add_version(v3, v4, 400, b"fourth");
        server.mock_add_version(v4, v5, 500, b"fifth");
        server.mock_add_version(v5, v6, 600, b"sixth");
        server.mock_set_latest(v6);

        let mut expected = server.empty_clone();
        expected.mock_add_snapshot(v3, 301, b"snap 1");
        expected.mock_add_version(v3, v4, 400, b"fourth");
        expected.mock_add_version(v4, v5, 500, b"fifth");
        expected.mock_add_version(v5, v6, 600, b"sixth");
        expected.mock_set_latest(v6);

        assert_ne!(server.unencrypted(), expected.unencrypted());
        server.cleanup().unwrap();
        assert_eq!(server.unencrypted(), expected.unencrypted());
    }

    #[test]
    fn cleanup_children_of_latest() {
        // New versions that are children of the latest version are not cleaned up.
        let mut server = make_server();
        let (v1, v2, v3) = (Uuid::new_v4(), Uuid::new_v4(), Uuid::new_v4());
        let (vnew1, vnew2) = (Uuid::new_v4(), Uuid::new_v4());
        server.mock_add_version(v1, v2, 1000, b"second");
        server.mock_add_version(v2, v3, 1000, b"third");
        server.mock_add_version(v3, vnew1, 1000, b"new 1");
        server.mock_add_version(v3, vnew2, 1000, b"new 2");
        // Two replicas are adding new versions, but v3 is still latest.
        server.mock_set_latest(v3);

        let expected = server.clone();

        server.cleanup().unwrap();
        assert_eq!(server.unencrypted(), expected.unencrypted());
    }

    #[test]
    fn add_snapshot() {
        let mut server = make_server();
        let v = Uuid::new_v4();

        let mut expected = server.empty_clone();
        expected.mock_add_snapshot(v, INSERTION_TIME, b"SNAP");

        assert_ne!(server.unencrypted(), expected.unencrypted());
        server.last_version_added = Some(v);
        server.add_snapshot(v, b"SNAP".to_vec()).unwrap();
        assert_eq!(server.unencrypted(), expected.unencrypted());
    }

    #[test]
    fn add_snapshot_wrong_version() {
        let mut server = make_server();
        let v = Uuid::new_v4();
        server.last_version_added = Some(Uuid::new_v4());
        assert!(server.add_snapshot(v, b"SNAP".to_vec()).is_err());
    }

    #[test]
    fn get_snapshot_missing() {
        let mut server = make_server();
        assert_eq!(server.get_snapshot().unwrap(), None);
    }

    #[test]
    fn get_snapshot_present() {
        let mut server = make_server();
        let v = Uuid::new_v4();
        server.mock_add_snapshot(v, 1000, b"SNAP");
        assert_eq!(server.get_snapshot().unwrap(), Some((v, b"SNAP".to_vec())));
    }
}
