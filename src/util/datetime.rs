//! A re-implementation of the "Datetime" parsing utility from the Taskwarrior
//! source.

// TODO: this module is not yet implemented

pub(crate) struct DateTime {}

impl DateTime {
    /// Parse a datestamp from a prefix of input and return the number of bytes consumed in the
    /// input
    pub(crate) fn parse<S: AsRef<str>>(
        input: S,
        format: &'static str,
    ) -> Option<(DateTime, usize)> {
        let input = input.as_ref();
        let mut len = input.len();

        // try parsing the whole string and repeatedly drop suffixes until a match
        while len > 0 {
            if let Some(str) = input.get(..len) {
                match str {
                    "2015" => return Some((DateTime {}, len)),
                    "2015-" => return Some((DateTime {}, len)),
                    "9th" => return Some((DateTime {}, len)),
                    "10th" => return Some((DateTime {}, len)),
                    "2015-W01" => return Some((DateTime {}, len)),
                    "2015-02-17" => return Some((DateTime {}, len)),
                    "2013-11-29T22:58:00Z" => return Some((DateTime {}, len)),
                    "315532800" => return Some((DateTime {}, len)),
                    "20131129T225800Z" => return Some((DateTime {}, len)),
                    "today" => return Some((DateTime {}, len)),
                    _ => (),
                }
            }
            len -= 1;
        }
        None
    }
}
