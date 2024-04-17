use crate::errors::{Error, Result};
use crate::server::{
    AddVersionResult, GetVersionResult, HistorySegment, Server, Snapshot, SnapshotUrgency,
    VersionId,
};
use std::time::Duration;
use url::Url;
use uuid::Uuid;

use super::encryption::{Cryptor, Sealed, Secret, Unsealed};

pub struct SyncServer {
    origin: String,
    client_id: Uuid,
    cryptor: Cryptor,
    agent: ureq::Agent,
}

/// The content-type for history segments (opaque blobs of bytes)
const HISTORY_SEGMENT_CONTENT_TYPE: &str = "application/vnd.taskchampion.history-segment";

/// The content-type for snapshots (opaque blobs of bytes)
const SNAPSHOT_CONTENT_TYPE: &str = "application/vnd.taskchampion.snapshot";

/// A SyncServer communicates with a sync server over HTTP.
impl SyncServer {
    /// Construct a new SyncServer.  The `origin` is the sync server's protocol and hostname
    /// without a trailing slash, such as `https://tcsync.example.com`.  Pass a client_id to
    /// identify this client to the server.  Multiple replicas synchronizing the same task history
    /// should use the same client_id.
    pub fn new(origin: String, client_id: Uuid, encryption_secret: Vec<u8>) -> Result<SyncServer> {
        let origin = Url::parse(&origin)
            .map_err(|_| Error::Server(format!("Could not parse {} as a URL", origin)))?;
        if origin.path() != "/" {
            return Err(Error::Server(format!(
                "Server origin must have an empty path; got {}",
                origin
            )));
        }
        Ok(SyncServer {
            origin: origin.to_string(),
            client_id,
            cryptor: Cryptor::new(client_id, &Secret(encryption_secret.to_vec()))?,
            agent: ureq::AgentBuilder::new()
                .timeout_connect(Duration::from_secs(10))
                .timeout_read(Duration::from_secs(60))
                .build(),
        })
    }
}

/// Read a UUID-bearing header or fail trying
fn get_uuid_header(resp: &ureq::Response, name: &str) -> Result<Uuid> {
    let value = resp
        .header(name)
        .ok_or_else(|| anyhow::anyhow!("Response does not have {} header", name))?;
    let value = Uuid::parse_str(value)
        .map_err(|e| anyhow::anyhow!("{} header is not a valid UUID: {}", name, e))?;
    Ok(value)
}

/// Read the X-Snapshot-Request header and return a SnapshotUrgency
fn get_snapshot_urgency(resp: &ureq::Response) -> SnapshotUrgency {
    match resp.header("X-Snapshot-Request") {
        None => SnapshotUrgency::None,
        Some(hdr) => match hdr {
            "urgency=low" => SnapshotUrgency::Low,
            "urgency=high" => SnapshotUrgency::High,
            _ => SnapshotUrgency::None,
        },
    }
}

fn sealed_from_resp(resp: ureq::Response, version_id: Uuid, content_type: &str) -> Result<Sealed> {
    use std::io::Read;
    if resp.header("Content-Type") == Some(content_type) {
        let mut reader = resp.into_reader();
        let mut payload = vec![];
        reader.read_to_end(&mut payload)?;
        Ok(Sealed {
            version_id,
            payload,
        })
    } else {
        Err(Error::Server(String::from(
            "Response did not have expected content-type",
        )))
    }
}

impl Server for SyncServer {
    fn add_version(
        &mut self,
        parent_version_id: VersionId,
        history_segment: HistorySegment,
    ) -> Result<(AddVersionResult, SnapshotUrgency)> {
        let url = format!("{}v1/client/add-version/{}", self.origin, parent_version_id);
        let unsealed = Unsealed {
            version_id: parent_version_id,
            payload: history_segment,
        };
        let sealed = self.cryptor.seal(unsealed)?;
        match self
            .agent
            .post(&url)
            .set("Content-Type", HISTORY_SEGMENT_CONTENT_TYPE)
            .set("X-Client-Id", &self.client_id.to_string())
            .send_bytes(sealed.as_ref())
        {
            Ok(resp) => {
                let version_id = get_uuid_header(&resp, "X-Version-Id")?;
                Ok((
                    AddVersionResult::Ok(version_id),
                    get_snapshot_urgency(&resp),
                ))
            }
            Err(ureq::Error::Status(status, resp)) if status == 409 => {
                let parent_version_id = get_uuid_header(&resp, "X-Parent-Version-Id")?;
                Ok((
                    AddVersionResult::ExpectedParentVersion(parent_version_id),
                    SnapshotUrgency::None,
                ))
            }
            Err(err) => Err(err.into()),
        }
    }

    fn get_child_version(&mut self, parent_version_id: VersionId) -> Result<GetVersionResult> {
        let url = format!(
            "{}v1/client/get-child-version/{}",
            self.origin, parent_version_id
        );
        match self
            .agent
            .get(&url)
            .set("X-Client-Id", &self.client_id.to_string())
            .call()
        {
            Ok(resp) => {
                let parent_version_id = get_uuid_header(&resp, "X-Parent-Version-Id")?;
                let version_id = get_uuid_header(&resp, "X-Version-Id")?;
                let sealed =
                    sealed_from_resp(resp, parent_version_id, HISTORY_SEGMENT_CONTENT_TYPE)?;
                let history_segment = self.cryptor.unseal(sealed)?.payload;
                Ok(GetVersionResult::Version {
                    version_id,
                    parent_version_id,
                    history_segment,
                })
            }
            Err(ureq::Error::Status(status, _)) if status == 404 => {
                Ok(GetVersionResult::NoSuchVersion)
            }
            Err(err) => Err(err.into()),
        }
    }

    fn add_snapshot(&mut self, version_id: VersionId, snapshot: Snapshot) -> Result<()> {
        let url = format!("{}v1/client/add-snapshot/{}", self.origin, version_id);
        let unsealed = Unsealed {
            version_id,
            payload: snapshot,
        };
        let sealed = self.cryptor.seal(unsealed)?;
        Ok(self
            .agent
            .post(&url)
            .set("Content-Type", SNAPSHOT_CONTENT_TYPE)
            .set("X-Client-Id", &self.client_id.to_string())
            .send_bytes(sealed.as_ref())
            .map(|_| ())?)
    }

    fn get_snapshot(&mut self) -> Result<Option<(VersionId, Snapshot)>> {
        let url = format!("{}v1/client/snapshot", self.origin);
        match self
            .agent
            .get(&url)
            .set("X-Client-Id", &self.client_id.to_string())
            .call()
        {
            Ok(resp) => {
                let version_id = get_uuid_header(&resp, "X-Version-Id")?;
                let sealed = sealed_from_resp(resp, version_id, SNAPSHOT_CONTENT_TYPE)?;
                let snapshot = self.cryptor.unseal(sealed)?.payload;
                Ok(Some((version_id, snapshot)))
            }
            Err(ureq::Error::Status(status, _)) if status == 404 => Ok(None),
            Err(err) => Err(err.into()),
        }
    }
}
