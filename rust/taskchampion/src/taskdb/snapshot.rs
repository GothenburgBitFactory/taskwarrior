use crate::storage::{StorageTxn, TaskMap, VersionId};
use flate2::{read::ZlibDecoder, write::ZlibEncoder, Compression};
use serde::de::{Deserialize, Deserializer, MapAccess, Visitor};
use serde::ser::{Serialize, SerializeMap, Serializer};
use std::fmt;
use uuid::Uuid;

/// A newtype to wrap the result of [`crate::storage::StorageTxn::all_tasks`]
pub(super) struct SnapshotTasks(Vec<(Uuid, TaskMap)>);

impl Serialize for SnapshotTasks {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let mut map = serializer.serialize_map(Some(self.0.len()))?;
        for (k, v) in &self.0 {
            map.serialize_entry(k, v)?;
        }
        map.end()
    }
}

struct TaskDbVisitor;

impl<'de> Visitor<'de> for TaskDbVisitor {
    type Value = SnapshotTasks;

    fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
        formatter.write_str("a map representing a task snapshot")
    }

    fn visit_map<M>(self, mut access: M) -> Result<Self::Value, M::Error>
    where
        M: MapAccess<'de>,
    {
        let mut map = SnapshotTasks(Vec::with_capacity(access.size_hint().unwrap_or(0)));

        while let Some((key, value)) = access.next_entry()? {
            map.0.push((key, value));
        }

        Ok(map)
    }
}

impl<'de> Deserialize<'de> for SnapshotTasks {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        deserializer.deserialize_map(TaskDbVisitor)
    }
}

impl SnapshotTasks {
    pub(super) fn encode(&self) -> anyhow::Result<Vec<u8>> {
        let mut encoder = ZlibEncoder::new(Vec::new(), Compression::default());
        serde_json::to_writer(&mut encoder, &self)?;
        Ok(encoder.finish()?)
    }

    pub(super) fn decode(snapshot: &[u8]) -> anyhow::Result<Self> {
        let decoder = ZlibDecoder::new(snapshot);
        Ok(serde_json::from_reader(decoder)?)
    }

    pub(super) fn into_inner(self) -> Vec<(Uuid, TaskMap)> {
        self.0
    }
}

/// Generate a snapshot (compressed, unencrypted) for the current state of the taskdb in the given
/// storage.
pub(super) fn make_snapshot(txn: &mut dyn StorageTxn) -> anyhow::Result<Vec<u8>> {
    let all_tasks = SnapshotTasks(txn.all_tasks()?);
    all_tasks.encode()
}

/// Apply the given snapshot (compressed, unencrypted) to the taskdb's storage.
pub(super) fn apply_snapshot(
    txn: &mut dyn StorageTxn,
    version: VersionId,
    snapshot: &[u8],
) -> anyhow::Result<()> {
    let all_tasks = SnapshotTasks::decode(snapshot)?;

    // double-check emptiness
    if !txn.is_empty()? {
        anyhow::bail!("Cannot apply snapshot to a non-empty task database");
    }

    for (uuid, task) in all_tasks.into_inner().drain(..) {
        txn.set_task(uuid, task)?;
    }
    txn.set_base_version(version)?;

    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::storage::{InMemoryStorage, Storage, TaskMap};
    use pretty_assertions::assert_eq;

    #[test]
    fn test_serialize_empty() -> anyhow::Result<()> {
        let empty = SnapshotTasks(vec![]);
        assert_eq!(serde_json::to_vec(&empty)?, b"{}".to_owned());
        Ok(())
    }

    #[test]
    fn test_serialize_tasks() -> anyhow::Result<()> {
        let u = Uuid::new_v4();
        let m: TaskMap = vec![("description".to_owned(), "my task".to_owned())]
            .drain(..)
            .collect();
        let all_tasks = SnapshotTasks(vec![(u, m)]);
        assert_eq!(
            serde_json::to_vec(&all_tasks)?,
            format!("{{\"{}\":{{\"description\":\"my task\"}}}}", u).into_bytes(),
        );
        Ok(())
    }

    #[test]
    fn test_round_trip() -> anyhow::Result<()> {
        let mut storage = InMemoryStorage::new();
        let version = Uuid::new_v4();

        let task1 = (
            Uuid::new_v4(),
            vec![("description".to_owned(), "one".to_owned())]
                .drain(..)
                .collect::<TaskMap>(),
        );
        let task2 = (
            Uuid::new_v4(),
            vec![("description".to_owned(), "two".to_owned())]
                .drain(..)
                .collect::<TaskMap>(),
        );

        {
            let mut txn = storage.txn()?;
            txn.set_task(task1.0, task1.1.clone())?;
            txn.set_task(task2.0, task2.1.clone())?;
            txn.commit()?;
        }

        let snap = {
            let mut txn = storage.txn()?;
            make_snapshot(txn.as_mut())?
        };

        // apply that snapshot to a fresh bit of fake
        let mut storage = InMemoryStorage::new();
        {
            let mut txn = storage.txn()?;
            apply_snapshot(txn.as_mut(), version, &snap)?;
            txn.commit()?
        }

        {
            let mut txn = storage.txn()?;
            assert_eq!(txn.get_task(task1.0)?, Some(task1.1));
            assert_eq!(txn.get_task(task2.0)?, Some(task2.1));
            assert_eq!(txn.all_tasks()?.len(), 2);
            assert_eq!(txn.base_version()?, version);
            assert_eq!(txn.operations()?.len(), 0);
            assert_eq!(txn.get_working_set()?.len(), 1);
        }

        Ok(())
    }
}
