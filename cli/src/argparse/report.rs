use super::{ArgList, Filter};
use nom::IResult;

/// A report specifies a filter as well as a sort order and information about which
/// task attributes to display
#[derive(Debug, PartialEq, Default)]
pub(crate) struct Report {
    pub filter: Filter,
}

impl Report {
    pub(super) fn parse(input: ArgList) -> IResult<ArgList, Report> {
        let (input, filter) = Filter::parse(input)?;
        Ok((input, Report { filter }))
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_empty() {
        let (input, report) = Report::parse(argv![]).unwrap();
        assert_eq!(input.len(), 0);
        assert_eq!(
            report,
            Report {
                ..Default::default()
            }
        );
    }
}
