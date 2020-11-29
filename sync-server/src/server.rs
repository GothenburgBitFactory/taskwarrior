//! This module implements the core logic of the server: handling transactions, upholding
//! invariants, and so on.
use crate::storage::{Client, StorageTxn};
use failure::Fallible;
use uuid::Uuid;

/// The distinguished value for "no version"
pub const NO_VERSION_ID: VersionId = Uuid::nil();

pub(crate) type HistorySegment = Vec<u8>;
pub(crate) type ClientId = Uuid;
pub(crate) type VersionId = Uuid;

/// Response to get_child_version
#[derive(Clone, PartialEq, Debug)]
pub(crate) struct GetVersionResult {
    pub(crate) version_id: Uuid,
    pub(crate) parent_version_id: Uuid,
    pub(crate) history_segment: HistorySegment,
}

pub(crate) fn get_child_version<'a>(
    mut txn: Box<dyn StorageTxn + 'a>,
    client_id: ClientId,
    parent_version_id: VersionId,
) -> Fallible<Option<GetVersionResult>> {
    Ok(txn
        .get_version_by_parent(client_id, parent_version_id)?
        .map(|version| GetVersionResult {
            version_id: version.version_id,
            parent_version_id: version.parent_version_id,
            history_segment: version.history_segment,
        }))
}

/// Response to add_version
#[derive(Clone, PartialEq, Debug)]
pub(crate) enum AddVersionResult {
    /// OK, version added with the given ID
    Ok(VersionId),
    /// Rejected; expected a version with the given parent version
    ExpectedParentVersion(VersionId),
}

pub(crate) fn add_version<'a>(
    mut txn: Box<dyn StorageTxn + 'a>,
    client_id: ClientId,
    client: Client,
    parent_version_id: VersionId,
    history_segment: HistorySegment,
) -> Fallible<AddVersionResult> {
    log::debug!(
        "add_version(client_id: {}, parent_version_id: {})",
        client_id,
        parent_version_id,
    );

    // check if this version is acceptable, under the protection of the transaction
    if client.latest_version_id != NO_VERSION_ID && parent_version_id != client.latest_version_id {
        log::debug!("add_version request rejected: mismatched latest_version_id");
        return Ok(AddVersionResult::ExpectedParentVersion(
            client.latest_version_id,
        ));
    }

    // invent a version ID
    let version_id = Uuid::new_v4();
    log::debug!(
        "add_version request accepted: new version_id: {}",
        version_id
    );

    // update the DB
    txn.add_version(client_id, version_id, parent_version_id, history_segment)?;
    txn.set_client_latest_version_id(client_id, version_id)?;
    txn.commit()?;

    Ok(AddVersionResult::Ok(version_id))
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::storage::{InMemoryStorage, Storage};

    #[test]
    fn gcv_not_found() -> Fallible<()> {
        let storage = InMemoryStorage::new();
        let txn = storage.txn()?;
        let client_id = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        assert_eq!(get_child_version(txn, client_id, parent_version_id)?, None);
        Ok(())
    }

    #[test]
    fn gcv_found() -> Fallible<()> {
        let storage = InMemoryStorage::new();
        let mut txn = storage.txn()?;
        let client_id = Uuid::new_v4();
        let version_id = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        let history_segment = b"abcd".to_vec();

        txn.add_version(
            client_id,
            version_id,
            parent_version_id,
            history_segment.clone(),
        )?;

        assert_eq!(
            get_child_version(txn, client_id, parent_version_id)?,
            Some(GetVersionResult {
                version_id,
                parent_version_id,
                history_segment,
            })
        );
        Ok(())
    }

    #[test]
    fn av_conflict() -> Fallible<()> {
        let storage = InMemoryStorage::new();
        let mut txn = storage.txn()?;
        let client_id = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        let history_segment = b"abcd".to_vec();
        let existing_parent_version_id = Uuid::new_v4();
        let client = Client {
            latest_version_id: existing_parent_version_id,
        };

        assert_eq!(
            add_version(
                txn,
                client_id,
                client,
                parent_version_id,
                history_segment.clone()
            )?,
            AddVersionResult::ExpectedParentVersion(existing_parent_version_id)
        );

        // verify that the storage wasn't updated
        txn = storage.txn()?;
        assert_eq!(txn.get_client(client_id)?, None);
        assert_eq!(
            txn.get_version_by_parent(client_id, parent_version_id)?,
            None
        );

        Ok(())
    }

    fn test_av_success(latest_version_id_nil: bool) -> Fallible<()> {
        let storage = InMemoryStorage::new();
        let mut txn = storage.txn()?;
        let client_id = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        let history_segment = b"abcd".to_vec();
        let latest_version_id = if latest_version_id_nil {
            Uuid::nil()
        } else {
            parent_version_id
        };

        txn.new_client(client_id, latest_version_id)?;
        let client = txn.get_client(client_id)?.unwrap();

        let result = add_version(
            txn,
            client_id,
            client,
            parent_version_id,
            history_segment.clone(),
        )?;
        if let AddVersionResult::Ok(new_version_id) = result {
            // check that it invented a new version ID
            assert!(new_version_id != parent_version_id);

            // verify that the storage was updated
            txn = storage.txn()?;
            let client = txn.get_client(client_id)?.unwrap();
            assert_eq!(client.latest_version_id, new_version_id);
            let version = txn
                .get_version_by_parent(client_id, parent_version_id)?
                .unwrap();
            assert_eq!(version.version_id, new_version_id);
            assert_eq!(version.parent_version_id, parent_version_id);
            assert_eq!(version.history_segment, history_segment);
        } else {
            panic!("did not get Ok from add_version");
        }

        Ok(())
    }

    #[test]
    fn av_success_with_existing_history() -> Fallible<()> {
        test_av_success(true)
    }

    #[test]
    fn av_success_nil_latest_version_id() -> Fallible<()> {
        test_av_success(false)
    }
}
