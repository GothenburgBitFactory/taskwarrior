/// The priority of a task
#[derive(Debug, PartialEq)]
pub enum Priority {
    /// Low
    L,
    /// Medium
    M,
    /// High
    H,
}

#[allow(dead_code)]
impl Priority {
    /// Get a Priority from the 1-character value in a TaskMap,
    /// defaulting to M
    pub(crate) fn from_taskmap(s: &str) -> Priority {
        match s {
            "L" => Priority::L,
            "M" => Priority::M,
            "H" => Priority::H,
            _ => Priority::M,
        }
    }

    /// Get the 1-character value for this priority to use in the TaskMap.
    pub(crate) fn to_taskmap(&self) -> &str {
        match self {
            Priority::L => "L",
            Priority::M => "M",
            Priority::H => "H",
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_priority() {
        assert_eq!(Priority::L.to_taskmap(), "L");
        assert_eq!(Priority::M.to_taskmap(), "M");
        assert_eq!(Priority::H.to_taskmap(), "H");
        assert_eq!(Priority::from_taskmap("L"), Priority::L);
        assert_eq!(Priority::from_taskmap("M"), Priority::M);
        assert_eq!(Priority::from_taskmap("H"), Priority::H);
    }
}
