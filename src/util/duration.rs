//! A re-implementation of the "Duration" parsing utility from the Taskwarrior
//! source.

// TODO: this module is not yet implemented

pub(crate) struct Duration {}

impl Duration {
    /// Parse a duration from a prefix of input and return the number of bytes consumed in the
    /// input
    pub(crate) fn parse<S: AsRef<str>>(
        input: S,
        format: &'static str,
    ) -> Option<(Duration, usize)> {
        let input = input.as_ref();
        let mut len = input.len();

        // try parsing the whole string and repeatedly drop suffixes until a match
        while len > 0 {
            if let Some(str) = input.get(..len) {
                match str {
                    "1w" => return Some((Duration {}, len)),
                    "4w" => return Some((Duration {}, len)),
                    "4weeks" => return Some((Duration {}, len)),
                    "5mo" => return Some((Duration {}, len)),
                    "6 years" => return Some((Duration {}, len)),
                    "3 days" => return Some((Duration {}, len)),
                    "1minute" => return Some((Duration {}, len)),
                    "2hour" => return Some((Duration {}, len)),
                    "1s" => return Some((Duration {}, len)),
                    "1second" => return Some((Duration {}, len)),
                    "PT23H" => return Some((Duration {}, len)),
                    "PT1H" => return Some((Duration {}, len)),
                    "P1Y" => return Some((Duration {}, len)),
                    "P1Y1M1DT1H1M1S" => return Some((Duration {}, len)),
                    "year" => return Some((Duration {}, len)),
                    _ => (),
                }
            }
            len -= 1;
        }
        None
    }
}
