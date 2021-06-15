use nom::{character::complete::*, combinator::*, sequence::*, IResult};
use std::convert::TryFrom;
use taskchampion::Tag;

/// Recognizes a tag prefixed with `+` and returns the tag value
pub(crate) fn plus_tag(input: &str) -> IResult<&str, Tag> {
    preceded(char('+'), map_res(rest, Tag::try_from))(input)
}

/// Recognizes a tag prefixed with `-` and returns the tag value
pub(crate) fn minus_tag(input: &str) -> IResult<&str, Tag> {
    preceded(char('-'), map_res(rest, Tag::try_from))(input)
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_plus_tag() {
        assert_eq!(plus_tag("+abc").unwrap().1, tag!("abc"));
        assert_eq!(plus_tag("+abc123").unwrap().1, tag!("abc123"));
        assert!(plus_tag("-abc123").is_err());
        assert!(plus_tag("+1abc").is_err());
    }

    #[test]
    fn test_minus_tag() {
        assert_eq!(minus_tag("-abc").unwrap().1, tag!("abc"));
        assert_eq!(minus_tag("-abc123").unwrap().1, tag!("abc123"));
        assert!(minus_tag("+abc123").is_err());
        assert!(minus_tag("-1abc").is_err());
    }
}
