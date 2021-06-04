use nom::bytes::complete::tag as nomtag;
use nom::{character::complete::*, combinator::*, sequence::*, IResult};

/// Recognizes any argument
pub(crate) fn any(input: &str) -> IResult<&str, &str> {
    rest(input)
}

/// Recognizes a report name
pub(crate) fn report_name(input: &str) -> IResult<&str, &str> {
    all_consuming(recognize(pair(alpha1, alphanumeric0)))(input)
}

/// Recognizes a literal string
pub(crate) fn literal(literal: &'static str) -> impl Fn(&str) -> IResult<&str, &str> {
    move |input: &str| all_consuming(nomtag(literal))(input)
}

#[cfg(test)]
mod test {
    use super::super::*;
    use super::*;

    #[test]
    fn test_arg_matching() {
        assert_eq!(
            arg_matching(plus_tag)(argv!["+foo", "bar"]).unwrap(),
            (argv!["bar"], tag!("foo"))
        );
        assert!(arg_matching(plus_tag)(argv!["foo", "bar"]).is_err());
    }

    #[test]
    fn test_literal() {
        assert_eq!(literal("list")("list").unwrap().1, "list");
        assert!(literal("list")("listicle").is_err());
        assert!(literal("list")(" list ").is_err());
        assert!(literal("list")("LiSt").is_err());
        assert!(literal("list")("denylist").is_err());
    }
}
