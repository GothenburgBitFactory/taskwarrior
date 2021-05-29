use nom::{character::complete::*, combinator::*, sequence::*, IResult};
use std::convert::TryFrom;
use taskchampion::Tag;

/// Recognizes a tag prefixed with `+` and returns the tag value
pub(crate) fn plus_tag(input: &str) -> IResult<&str, &str> {
    fn to_tag(input: (char, &str)) -> Result<&str, ()> {
        Ok(input.1)
    }
    map_res(
        all_consuming(tuple((
            char('+'),
            recognize(verify(rest, |s: &str| Tag::try_from(s).is_ok())),
        ))),
        to_tag,
    )(input)
}

/// Recognizes a tag prefixed with `-` and returns the tag value
pub(crate) fn minus_tag(input: &str) -> IResult<&str, &str> {
    fn to_tag(input: (char, &str)) -> Result<&str, ()> {
        Ok(input.1)
    }
    map_res(
        all_consuming(tuple((
            char('-'),
            recognize(verify(rest, |s: &str| Tag::try_from(s).is_ok())),
        ))),
        to_tag,
    )(input)
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_plus_tag() {
        assert_eq!(plus_tag("+abc").unwrap().1, "abc");
        assert_eq!(plus_tag("+abc123").unwrap().1, "abc123");
        assert!(plus_tag("-abc123").is_err());
        assert!(plus_tag("+abc123  ").is_err());
        assert!(plus_tag("  +abc123").is_err());
        assert!(plus_tag("+1abc").is_err());
    }

    #[test]
    fn test_minus_tag() {
        assert_eq!(minus_tag("-abc").unwrap().1, "abc");
        assert_eq!(minus_tag("-abc123").unwrap().1, "abc123");
        assert!(minus_tag("+abc123").is_err());
        assert!(minus_tag("-abc123  ").is_err());
        assert!(minus_tag("  -abc123").is_err());
        assert!(minus_tag("-1abc").is_err());
    }
}
