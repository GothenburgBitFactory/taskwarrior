use super::{any, timestamp};
use crate::argparse::NOW;
use chrono::prelude::*;
use nom::bytes::complete::tag as nomtag;
use nom::{branch::*, character::complete::*, combinator::*, sequence::*, IResult};
use taskchampion::Status;

/// Recognizes up to the colon of the common `<prefix>:...` syntax
fn colon_prefix(prefix: &'static str) -> impl Fn(&str) -> IResult<&str, &str> {
    fn to_suffix<'a>(input: (&'a str, char, &'a str)) -> Result<&'a str, ()> {
        Ok(input.2)
    }
    move |input: &str| {
        map_res(
            all_consuming(tuple((nomtag(prefix), char(':'), any))),
            to_suffix,
        )(input)
    }
}

/// Recognizes `status:{pending,completed,deleted}`
pub(crate) fn status_colon(input: &str) -> IResult<&str, Status> {
    fn to_status(input: &str) -> Result<Status, ()> {
        match input {
            "pending" => Ok(Status::Pending),
            "completed" => Ok(Status::Completed),
            "deleted" => Ok(Status::Deleted),
            _ => Err(()),
        }
    }
    map_res(colon_prefix("status"), to_status)(input)
}

/// Recognizes `wait:` to None and `wait:<ts>` to `Some(ts)`
pub(crate) fn wait_colon(input: &str) -> IResult<&str, Option<DateTime<Utc>>> {
    fn to_wait(input: DateTime<Utc>) -> Result<Option<DateTime<Utc>>, ()> {
        Ok(Some(input))
    }
    fn to_none(_: &str) -> Result<Option<DateTime<Utc>>, ()> {
        Ok(None)
    }
    preceded(
        nomtag("wait:"),
        alt((
            map_res(timestamp(*NOW, Local), to_wait),
            map_res(nomtag(""), to_none),
        )),
    )(input)
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn test_colon_prefix() {
        assert_eq!(colon_prefix("foo")("foo:abc").unwrap().1, "abc");
        assert_eq!(colon_prefix("foo")("foo:").unwrap().1, "");
        assert!(colon_prefix("foo")("foo").is_err());
    }

    #[test]
    fn test_status_colon() {
        assert_eq!(status_colon("status:pending").unwrap().1, Status::Pending);
        assert_eq!(
            status_colon("status:completed").unwrap().1,
            Status::Completed
        );
        assert_eq!(status_colon("status:deleted").unwrap().1, Status::Deleted);
        assert!(status_colon("status:foo").is_err());
        assert!(status_colon("status:complete").is_err());
        assert!(status_colon("status").is_err());
    }

    #[test]
    fn test_wait() {
        assert_eq!(wait_colon("wait:").unwrap(), ("", None));

        let one_day = *NOW + chrono::Duration::days(1);
        assert_eq!(wait_colon("wait:1d").unwrap(), ("", Some(one_day)));

        let one_day = *NOW + chrono::Duration::days(1);
        assert_eq!(wait_colon("wait:1d2").unwrap(), ("2", Some(one_day)));
    }
}
