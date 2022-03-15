use nom::{branch::*, character::complete::*, combinator::*, multi::*, sequence::*, IResult};
use taskchampion::Uuid;

/// A task identifier, as given in a filter command-line expression
#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub(crate) enum TaskId {
    /// A small integer identifying a working-set task
    WorkingSetId(usize),

    /// A full Uuid specifically identifying a task
    Uuid(Uuid),

    /// A prefix of a Uuid
    PartialUuid(String),
}

/// Recognizes a comma-separated list of TaskIds
pub(crate) fn id_list(input: &str) -> IResult<&str, Vec<TaskId>> {
    fn hex_n(n: usize) -> impl Fn(&str) -> IResult<&str, &str> {
        move |input: &str| recognize(many_m_n(n, n, one_of(&b"0123456789abcdefABCDEF"[..])))(input)
    }
    fn uuid(input: &str) -> Result<TaskId, ()> {
        Ok(TaskId::Uuid(Uuid::parse_str(input).map_err(|_| ())?))
    }
    fn partial_uuid(input: &str) -> Result<TaskId, ()> {
        Ok(TaskId::PartialUuid(input.to_owned()))
    }
    fn working_set_id(input: &str) -> Result<TaskId, ()> {
        Ok(TaskId::WorkingSetId(input.parse().map_err(|_| ())?))
    }
    all_consuming(separated_list1(
        char(','),
        alt((
            map_res(
                recognize(tuple((
                    hex_n(8),
                    char('-'),
                    hex_n(4),
                    char('-'),
                    hex_n(4),
                    char('-'),
                    hex_n(4),
                    char('-'),
                    hex_n(12),
                ))),
                uuid,
            ),
            map_res(
                recognize(tuple((
                    hex_n(8),
                    char('-'),
                    hex_n(4),
                    char('-'),
                    hex_n(4),
                    char('-'),
                    hex_n(4),
                ))),
                partial_uuid,
            ),
            map_res(
                recognize(tuple((hex_n(8), char('-'), hex_n(4), char('-'), hex_n(4)))),
                partial_uuid,
            ),
            map_res(
                recognize(tuple((hex_n(8), char('-'), hex_n(4)))),
                partial_uuid,
            ),
            map_res(hex_n(8), partial_uuid),
            // note that an 8-decimal-digit value will be treated as a UUID
            map_res(digit1, working_set_id),
        )),
    ))(input)
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn test_id_list_single() {
        assert_eq!(id_list("123").unwrap().1, vec![TaskId::WorkingSetId(123)]);
    }

    #[test]
    fn test_id_list_uuids() {
        assert_eq!(
            id_list("12341234").unwrap().1,
            vec![TaskId::PartialUuid(s!("12341234"))]
        );
        assert_eq!(
            id_list("1234abcd").unwrap().1,
            vec![TaskId::PartialUuid(s!("1234abcd"))]
        );
        assert_eq!(
            id_list("abcd1234").unwrap().1,
            vec![TaskId::PartialUuid(s!("abcd1234"))]
        );
        assert_eq!(
            id_list("abcd1234-1234").unwrap().1,
            vec![TaskId::PartialUuid(s!("abcd1234-1234"))]
        );
        assert_eq!(
            id_list("abcd1234-1234-2345").unwrap().1,
            vec![TaskId::PartialUuid(s!("abcd1234-1234-2345"))]
        );
        assert_eq!(
            id_list("abcd1234-1234-2345-3456").unwrap().1,
            vec![TaskId::PartialUuid(s!("abcd1234-1234-2345-3456"))]
        );
        assert_eq!(
            id_list("abcd1234-1234-2345-3456-0123456789ab").unwrap().1,
            vec![TaskId::Uuid(
                Uuid::parse_str("abcd1234-1234-2345-3456-0123456789ab").unwrap()
            )]
        );
    }

    #[test]
    fn test_id_list_invalid_partial_uuids() {
        assert!(id_list("abcd123").is_err());
        assert!(id_list("abcd12345").is_err());
        assert!(id_list("abcd1234-").is_err());
        assert!(id_list("abcd1234-123").is_err());
        assert!(id_list("abcd1234-1234-").is_err());
        assert!(id_list("abcd1234-12345-").is_err());
        assert!(id_list("abcd1234-1234-2345-3456-0123456789ab-").is_err());
    }

    #[test]
    fn test_id_list_uuids_mixed() {
        assert_eq!(id_list("abcd1234,abcd1234-1234,abcd1234-1234-2345,abcd1234-1234-2345-3456,abcd1234-1234-2345-3456-0123456789ab").unwrap().1,
        vec![TaskId::PartialUuid(s!("abcd1234")),
            TaskId::PartialUuid(s!("abcd1234-1234")),
            TaskId::PartialUuid(s!("abcd1234-1234-2345")),
            TaskId::PartialUuid(s!("abcd1234-1234-2345-3456")),
            TaskId::Uuid(Uuid::parse_str("abcd1234-1234-2345-3456-0123456789ab").unwrap()),
        ]);
    }
}
