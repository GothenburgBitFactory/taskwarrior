use crate::argparse::ArgList;
use nom::{
    error::{Error, ErrorKind},
    Err, IResult,
};

/// Consume a single argument from an argument list that matches the given string parser (one
/// of the other functions in this module).  The given parser must consume the entire input.
pub(crate) fn arg_matching<'a, O, F>(f: F) -> impl Fn(ArgList<'a>) -> IResult<ArgList, O>
where
    F: Fn(&'a str) -> IResult<&'a str, O>,
{
    move |input: ArgList<'a>| {
        if let Some(arg) = input.get(0) {
            return match f(arg) {
                Ok(("", rv)) => Ok((&input[1..], rv)),
                // single-arg parsers must consume the entire arg
                Ok((unconsumed, _)) => panic!("unconsumed argument input {}", unconsumed),
                // single-arg parsers are all complete parsers
                Err(Err::Incomplete(_)) => unreachable!(),
                // for error and failure, rewrite to an error at this position in the arugment list
                Err(Err::Error(Error { input: _, code })) => Err(Err::Error(Error { input, code })),
                Err(Err::Failure(Error { input: _, code })) => {
                    Err(Err::Failure(Error { input, code }))
                }
            };
        }

        Err(Err::Error(Error {
            input,
            // since we're using nom's built-in Error, our choices here are limited, but tihs
            // occurs when there's no argument where one is expected, so Eof seems appropriate
            code: ErrorKind::Eof,
        }))
    }
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
}
