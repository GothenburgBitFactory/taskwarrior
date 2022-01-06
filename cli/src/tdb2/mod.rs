//! TDB2 is TaskWarrior's on-disk database format.  The set of tasks is represented in
//! `pending.data` and `completed.data`.  There are other `.data` files as well, but those are not
//! used in TaskChampion.
use nom::{branch::*, character::complete::*, combinator::*, multi::*, sequence::*, IResult};
use std::fmt;

#[derive(Debug, Clone, PartialEq)]
pub(crate) struct File {
    pub(crate) lines: Vec<Line>,
}

#[derive(Clone, PartialEq)]
pub(crate) struct Line {
    pub(crate) attrs: Vec<Attr>,
}

#[derive(Clone, PartialEq)]
pub(crate) struct Attr {
    pub(crate) name: String,
    pub(crate) value: String,
}

impl File {
    pub(crate) fn from_str(input: &str) -> Result<File, ()> {
        Ok(File::parse(input).map(|(_, res)| res).map_err(|_| ())?)
    }

    fn parse(input: &str) -> IResult<&str, File> {
        all_consuming(fold_many0(
            terminated(Line::parse, char('\n')),
            File { lines: vec![] },
            |mut file, line| {
                file.lines.push(line);
                file
            },
        ))(input)
    }
}

impl Line {
    /// Parse a line in a TDB2 file.  See TaskWarrior's Task::Parse.
    fn parse(input: &str) -> IResult<&str, Line> {
        fn to_line(input: Vec<Attr>) -> Result<Line, ()> {
            Ok(Line { attrs: input })
        }
        map_res(
            delimited(
                char('['),
                separated_list0(char(' '), Attr::parse),
                char(']'),
            ),
            to_line,
        )(input)
    }
}

impl fmt::Debug for Line {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str("line!")?;
        f.debug_list().entries(self.attrs.iter()).finish()
    }
}

impl Attr {
    /// Parse an attribute (name-value pair).
    fn parse(input: &str) -> IResult<&str, Attr> {
        fn to_attr(input: (&str, String)) -> Result<Attr, ()> {
            Ok(Attr {
                name: input.0.into(),
                value: input.1,
            })
        }
        map_res(
            separated_pair(Attr::parse_name, char(':'), Attr::parse_value),
            to_attr,
        )(input)
    }

    /// Parse an attribute name, which is composed of any character but `:`.
    fn parse_name(input: &str) -> IResult<&str, &str> {
        recognize(many1(none_of(":")))(input)
    }

    /// Parse and interpret a quoted string.  Note that this does _not_ reverse the effects of

    fn parse_value(input: &str) -> IResult<&str, String> {
        // For the parsing part of the job, see Pig::getQuoted in TaskWarrior's libshared, which
        // merely finds the end of a string.
        //
        // The interpretation is defined in json::decode in libshared.  Fortunately, the data we
        // are reading was created with json::encode, which does not perform unicode escaping.

        fn escaped_string_char(input: &str) -> IResult<&str, char> {
            alt((
                // reverse the escaping performed in json::encode
                preceded(
                    char('\\'),
                    alt((
                        // some characters are simply escaped
                        one_of(r#""\/"#),
                        // others translate to control characters
                        value('\x08', char('b')),
                        value('\x0c', char('f')),
                        value('\n', char('n')),
                        value('\r', char('r')),
                        value('\t', char('t')),
                    )),
                ),
                // not a backslash or double-quote
                none_of("\"\\"),
            ))(input)
        }

        let inner = fold_many0(
            escaped_string_char,
            String::new(),
            |mut string, fragment| {
                string.push(fragment);
                string
            },
        );

        delimited(char('"'), inner, char('"'))(input)
    }
}

impl fmt::Debug for Attr {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_fmt(format_args!("{:?} => {:?}", self.name, self.value))
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;

    macro_rules! line {
        ($($n:expr => $v:expr),* $(,)?) => (
            Line{attrs: vec![$(Attr{name: $n.into(), value: $v.into()}),*]}
        );
    }

    #[test]
    fn file() {
        assert_eq!(
            File::parse(include_str!("test.data")).unwrap(),
            (
                "",
                File {
                    lines: vec![
                        line![
                            "description" => "snake 🐍",
                            "entry" => "1641670385",
                            "modified" => "1641670385",
                            "priority" => "M",
                            "status" => "pending",
                            "uuid" => "f19086c2-1f8d-4a6c-9b8d-f94901fb8e62",
                        ],
                        line![
                            "annotation_1585711454" =>
                                "https://blog.tensorflow.org/2020/03/face-and-hand-tracking-in-browser-with-mediapipe-and-tensorflowjs.html?linkId=83993617",
                            "description" => "try facemesh",
                            "entry" => "1585711451",
                            "modified" => "1592947544",
                            "priority" => "M",
                            "project" => "lists",
                            "status" => "pending",
                            "tags" => "idea",
                            "tags_idea" => "x",
                            "uuid" => "ee855dc7-6f61-408c-bc95-ebb52f7d529c",
                        ],
                        line![
                            "description" => "testing",
                            "entry" => "1554074416",
                            "modified" => "1554074416",
                            "priority" => "M",
                            "status" => "pending",
                            "uuid" => "4578fb67-359b-4483-afe4-fef15925ccd6",
                        ],
                        line![
                            "description" => "testing2",
                            "entry" => "1576352411",
                            "modified" => "1576352411",
                            "priority" => "M",
                            "status" => "pending",
                            "uuid" => "f5982cca-2ea1-4bfd-832c-9bd571dc0743",
                        ],
                        line![
                            "description" => "new-task",
                            "entry" => "1576352696",
                            "modified" => "1576352696",
                            "priority" => "M",
                            "status" => "pending",
                            "uuid" => "cfee3170-f153-4075-aa1d-e20bcac2841b",
                        ],
                        line![
                            "description" => "foo",
                            "entry" => "1579398776",
                            "modified" => "1579398776",
                            "priority" => "M",
                            "status" => "pending",
                            "uuid" => "df74ea94-5122-44fa-965a-637412fbbffc",
                        ],
                    ]
                }
            )
        );
    }

    #[test]
    fn empty_line() {
        assert_eq!(Line::parse("[]").unwrap(), ("", line![]));
    }

    #[test]
    fn escaped_line() {
        assert_eq!(
            Line::parse(r#"[annotation_1585711454:"\"\\\"" abc:"xx"]"#).unwrap(),
            (
                "",
                line!["annotation_1585711454" => "\"\\\"", "abc" => "xx"]
            )
        );
    }

    #[test]
    fn escaped_line_backslash() {
        assert_eq!(
            Line::parse(r#"[abc:"xx" 123:"x\\x"]"#).unwrap(),
            ("", line!["abc" => "xx", "123" => "x\\x"])
        );
    }

    #[test]
    fn escaped_line_quote() {
        assert_eq!(
            Line::parse(r#"[abc:"xx" 123:"x\"x"]"#).unwrap(),
            ("", line!["abc" => "xx", "123" => "x\"x"])
        );
    }

    #[test]
    fn unicode_line() {
        assert_eq!(
            Line::parse(r#"[description:"snake 🐍" entry:"1641670385" modified:"1641670385" priority:"M" status:"pending" uuid:"f19086c2-1f8d-4a6c-9b8d-f94901fb8e62"]"#).unwrap(),
            ("", line![
                "description" => "snake 🐍",
                "entry" => "1641670385",
                "modified" => "1641670385",
                "priority" => "M",
                "status" => "pending",
                "uuid" => "f19086c2-1f8d-4a6c-9b8d-f94901fb8e62",
            ]));
    }

    #[test]
    fn backslashed_attr() {
        assert!(Attr::parse(r#"one:"\""#).is_err());
        assert_eq!(
            Attr::parse(r#"two:"\\""#).unwrap(),
            (
                "",
                Attr {
                    name: "two".into(),
                    value: r#"\"#.into(),
                }
            )
        );
        assert!(Attr::parse(r#"three:"\\\""#).is_err());
        assert_eq!(
            Attr::parse(r#"four:"\\\\""#).unwrap(),
            (
                "",
                Attr {
                    name: "four".into(),
                    value: r#"\\"#.into(),
                }
            )
        );
    }

    #[test]
    fn backslash_frontslash() {
        assert_eq!(
            Attr::parse(r#"front:"\/""#).unwrap(),
            (
                "",
                Attr {
                    name: "front".into(),
                    value: r#"/"#.into(),
                }
            )
        );
    }

    #[test]
    fn backslash_control_chars() {
        assert_eq!(
            Attr::parse(r#"control:"\b\f\n\r\t""#).unwrap(),
            (
                "",
                Attr {
                    name: "control".into(),
                    value: "\x08\x0c\x0a\x0d\x09".into(),
                }
            )
        );
    }

    #[test]
    fn url_attr() {
        assert_eq!(
            Attr::parse(r#"annotation_1585711454:"https:\/\/blog.tensorflow.org\/2020\/03\/""#)
                .unwrap(),
            (
                "",
                Attr {
                    name: "annotation_1585711454".into(),
                    value: "https://blog.tensorflow.org/2020/03/".into(),
                }
            )
        );
    }
}
