use uuid::Uuid;

/// DependencyMap stores information on task dependencies between pending tasks.
///
/// This information requires a scan of the working set to generate, so it is
/// typically calculated once and re-used.
#[derive(Debug, PartialEq)]
pub struct DependencyMap {
    /// Edges of the dependency graph.  If (a, b) is in this array, then task a depends on tsak b.
    edges: Vec<(Uuid, Uuid)>,
}

impl DependencyMap {
    /// Create a new, empty DependencyMap.
    pub(super) fn new() -> Self {
        Self { edges: Vec::new() }
    }

    /// Add a dependency of a on b.
    pub(super) fn add_dependency(&mut self, a: Uuid, b: Uuid) {
        self.edges.push((a, b));
    }

    /// Return an iterator of Uuids on which task `deps_of` depends.  This is equivalent to
    /// `task.get_dependencies()`.
    pub fn dependencies(&self, dep_of: Uuid) -> impl Iterator<Item = Uuid> + '_ {
        self.edges
            .iter()
            .filter_map(move |(a, b)| if a == &dep_of { Some(*b) } else { None })
    }

    /// Return an iterator of Uuids of tasks that depend on `dep_on`
    /// `task.get_dependencies()`.
    pub fn dependents(&self, dep_on: Uuid) -> impl Iterator<Item = Uuid> + '_ {
        self.edges
            .iter()
            .filter_map(move |(a, b)| if b == &dep_on { Some(*a) } else { None })
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;
    use std::collections::HashSet;

    #[test]
    fn dependencies() {
        let t = Uuid::new_v4();
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();
        let mut dm = DependencyMap::new();

        dm.add_dependency(t, uuid1);
        dm.add_dependency(t, uuid2);
        dm.add_dependency(Uuid::new_v4(), t);
        dm.add_dependency(Uuid::new_v4(), uuid1);
        dm.add_dependency(uuid2, Uuid::new_v4());

        assert_eq!(
            dm.dependencies(t).collect::<HashSet<_>>(),
            set![uuid1, uuid2]
        );
    }

    #[test]
    fn dependents() {
        let t = Uuid::new_v4();
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();
        let mut dm = DependencyMap::new();

        dm.add_dependency(uuid1, t);
        dm.add_dependency(uuid2, t);
        dm.add_dependency(t, Uuid::new_v4());
        dm.add_dependency(Uuid::new_v4(), uuid1);
        dm.add_dependency(uuid2, Uuid::new_v4());

        assert_eq!(dm.dependents(t).collect::<HashSet<_>>(), set![uuid1, uuid2]);
    }
}
