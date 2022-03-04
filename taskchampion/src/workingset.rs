use std::collections::HashMap;
use uuid::Uuid;

/// A WorkingSet represents a snapshot of the working set from a replica.
///
/// A replica's working set is a mapping from small integers to task uuids for all pending tasks.
/// The small integers are meant to be stable, easily-typed identifiers for users to interact with
/// important tasks.
///
/// IMPORTANT: the content of the working set may change at any time that a DB transaction is not
/// in progress, and the data in this type will not be updated automatically.  It is up to the
/// caller to decide how long to keep this value, and how much to trust the accuracy of its
/// contents.  In practice, the answers are usually "a few milliseconds" and treating unexpected
/// results as non-fatal.
pub struct WorkingSet {
    by_index: Vec<Option<Uuid>>,
    by_uuid: HashMap<Uuid, usize>,
}

impl WorkingSet {
    /// Create a new WorkingSet.  Typically this is acquired via `replica.working_set()`
    pub(crate) fn new(by_index: Vec<Option<Uuid>>) -> Self {
        let mut by_uuid = HashMap::new();

        // working sets are 1-indexed, so element 0 should always be None
        assert!(by_index.is_empty() || by_index[0].is_none());

        for (index, uuid) in by_index.iter().enumerate() {
            if let Some(uuid) = uuid {
                by_uuid.insert(*uuid, index);
            }
        }
        Self { by_index, by_uuid }
    }

    /// Get the "length" of the working set: the total number of uuids in the set.
    pub fn len(&self) -> usize {
        self.by_index.iter().filter(|e| e.is_some()).count()
    }

    /// Get the largest index in the working set, or zero if the set is empty.
    pub fn largest_index(&self) -> usize {
        self.by_index.len().saturating_sub(1)
    }

    /// True if the length is zero
    pub fn is_empty(&self) -> bool {
        self.by_index.iter().all(|e| e.is_none())
    }

    /// Get the uuid with the given index, if any exists.
    pub fn by_index(&self, index: usize) -> Option<Uuid> {
        if let Some(Some(uuid)) = self.by_index.get(index) {
            Some(*uuid)
        } else {
            None
        }
    }

    /// Get the index for the given uuid, if any
    pub fn by_uuid(&self, uuid: Uuid) -> Option<usize> {
        self.by_uuid.get(&uuid).copied()
    }

    /// Iterate over pairs (index, uuid), in order by index.
    pub fn iter(&self) -> impl Iterator<Item = (usize, Uuid)> + '_ {
        self.by_index
            .iter()
            .enumerate()
            .filter_map(|(index, uuid)| uuid.as_ref().map(|uuid| (index, *uuid)))
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;

    fn make() -> (Uuid, Uuid, WorkingSet) {
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();
        (
            uuid1,
            uuid2,
            WorkingSet::new(vec![None, Some(uuid1), None, Some(uuid2), None]),
        )
    }

    #[test]
    fn test_new() {
        let (_, uuid2, ws) = make();
        assert_eq!(ws.by_index[3], Some(uuid2));
        assert_eq!(ws.by_uuid.get(&uuid2), Some(&3));
    }

    #[test]
    fn test_len_and_is_empty() {
        let (_, _, ws) = make();
        assert_eq!(ws.len(), 2);
        assert_eq!(ws.is_empty(), false);

        let ws = WorkingSet::new(vec![]);
        assert_eq!(ws.len(), 0);
        assert_eq!(ws.is_empty(), true);

        let ws = WorkingSet::new(vec![None, None, None]);
        assert_eq!(ws.len(), 0);
        assert_eq!(ws.is_empty(), true);
    }

    #[test]
    fn test_largest_index() {
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();

        let ws = WorkingSet::new(vec![]);
        assert_eq!(ws.largest_index(), 0);

        let ws = WorkingSet::new(vec![None, Some(uuid1)]);
        assert_eq!(ws.largest_index(), 1);

        let ws = WorkingSet::new(vec![None, Some(uuid1), None, Some(uuid2)]);
        assert_eq!(ws.largest_index(), 3);

        let ws = WorkingSet::new(vec![None, Some(uuid1), None, Some(uuid2), None]);
        assert_eq!(ws.largest_index(), 4);
    }

    #[test]
    fn test_by_index() {
        let (uuid1, uuid2, ws) = make();
        assert_eq!(ws.by_index(0), None);
        assert_eq!(ws.by_index(1), Some(uuid1));
        assert_eq!(ws.by_index(2), None);
        assert_eq!(ws.by_index(3), Some(uuid2));
        assert_eq!(ws.by_index(4), None);
        assert_eq!(ws.by_index(100), None); // past the end of the vector
    }

    #[test]
    fn test_by_uuid() {
        let (uuid1, uuid2, ws) = make();
        let nosuch = Uuid::new_v4();
        assert_eq!(ws.by_uuid(uuid1), Some(1));
        assert_eq!(ws.by_uuid(uuid2), Some(3));
        assert_eq!(ws.by_uuid(nosuch), None);
    }

    #[test]
    fn test_iter() {
        let (uuid1, uuid2, ws) = make();
        assert_eq!(ws.iter().collect::<Vec<_>>(), vec![(1, uuid1), (3, uuid2),]);
    }
}
