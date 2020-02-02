//! A re-implementation of TaskWarrior's Lexer.
//!
//! This is tested to pass that module's tests, and includes some additional tests that were also
//! verified against that module.

use crate::util::datetime::DateTime;
use crate::util::duration::Duration;
use std::convert::TryFrom;

const UUID_PATTERN: &[u8] = b"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
const UUID_MIN_LENGTH: usize = 8;
const MINIMUM_MATCH_LEN: usize = 3;
const DATE_SUBELEMENTS: &[&str] = &[
    "year", "month", "day", "week", "weekday", "julian", "hour", "minute", "second",
];

#[derive(PartialEq, Debug, Clone, Copy)]
pub(crate) enum Type {
    Uuid,
    Number,
    Hex,
    String,
    URL,
    Pair,
    Set,
    Separator,
    Tag,
    Path,
    Substitution,
    Pattern,
    Op,
    DOM,
    Identifier,
    Word,
    Date,
    Duration,
}

pub(crate) struct Lexer {
    text: String,
    cursor: usize,
    eos: usize,
    attributes: Vec<String>,
}

// TaskWarrior uses some non-standard character definitions, so they are repeated verbatim here,
// rather than defaulting to the unicode functions available on the char type.

/// Returns true if this character is whitespace, as defined in TaskWarrior's libshared.
fn unicode_whitespace(c: char) -> bool {
    unicode_horizontal_whitespace(c) || unicode_vertical_whitespace(c)
}

/// Returns true if this character is horizontal whitespace, as defined in TaskWarrior's libshared.
fn unicode_horizontal_whitespace(c: char) -> bool {
    let c: u32 = c.into();
    return c == 0x0020 ||   // space Common  Separator, space
          c == 0x0009 ||   // Common  Other, control  HT, Horizontal Tab
          c == 0x00A0 ||   // no-break space  Common  Separator, space
          c == 0x1680 ||   // ogham space mark  Ogham Separator, space
          c == 0x180E ||   // mongolian vowel separator Mongolian Separator, space
          c == 0x2000 ||   // en quad Common  Separator, space
          c == 0x2001 ||   // em quad Common  Separator, space
          c == 0x2002 ||   // en space  Common  Separator, space
          c == 0x2003 ||   // em space  Common  Separator, space
          c == 0x2004 ||   // three-per-em space  Common  Separator, space
          c == 0x2005 ||   // four-per-em space Common  Separator, space
          c == 0x2006 ||   // six-per-em space  Common  Separator, space
          c == 0x2007 ||   // figure space  Common  Separator, space
          c == 0x2008 ||   // punctuation space Common  Separator, space
          c == 0x2009 ||   // thin space  Common  Separator, space
          c == 0x200A ||   // hair space  Common  Separator, space
          c == 0x200B ||   // zero width space
          c == 0x200C ||   // zero width non-joiner
          c == 0x200D ||   // zero width joiner
          c == 0x202F ||   // narrow no-break space Common  Separator, space
          c == 0x205F ||   // medium mathematical space Common  Separator, space
          c == 0x2060 ||   // word joiner
          c == 0x3000; // ideographic space Common  Separator, space
}

/// Returns true if this character is vertical whitespace, as defined in TaskWarrior's libshared.
fn unicode_vertical_whitespace(c: char) -> bool {
    let c: u32 = c.into();
    return c == 0x000A ||   // Common  Other, control  LF, Line feed
          c == 0x000B ||   // Common  Other, control  VT, Vertical Tab
          c == 0x000C ||   // Common  Other, control  FF, Form feed
          c == 0x000D ||   // Common  Other, control  CR, Carriage return
          c == 0x0085 ||   // Common  Other, control  NEL, Next line
          c == 0x2028 ||   // line separator  Common  Separator, line
          c == 0x2029; // paragraph separator Common  Separator, paragraph
}

/// Returns true if the given character is an ascii digit
fn unicode_latin_digit(c: char) -> bool {
    c.is_ascii_digit()
}

/// Returns true if the given character is an ascii letter
fn unicode_latin_alpha(c: char) -> bool {
    c.is_ascii_alphabetic()
}

/// Replicates the C function of the same name, which only recognizes ASCII printable
fn isprint(c: char) -> bool {
    c.is_ascii_graphic()
}

/// Returns true if the given character is punctuation.
fn is_punctuation(c: char) -> bool {
    isprint(c)
        && c != ' '
        && c != '@'
        && c != '#'
        && c != '$'
        && c != '_'
        && !unicode_latin_digit(c)
        && !unicode_latin_alpha(c)
}

/// Returns true if this character is an operator
fn is_single_char_operator(c: char) -> bool {
    match c {
        '+' | '-' | '*' | '/' | '(' | ')' | '<' | '>' | '^' | '!' | '%' | '=' | '~' => true,
        _ => false,
    }
}

/// Returns true if this character can start an identifier
fn is_identifier_start(c: char) -> bool {
    !unicode_whitespace(c)
        && !unicode_latin_digit(c)
        && !is_single_char_operator(c)
        && !is_punctuation(c)
}

/// Returns true if this character can be in the middle of an identifier
fn is_identifier_next(c: char) -> bool {
    c != ':' && c != '=' && !unicode_whitespace(c) && !is_single_char_operator(c)
}

/// Returns true if the sequence `<left><right>` represents a token boundary.
fn is_boundary(left: char, right: char) -> bool {
    right == '\0'
        || (unicode_latin_alpha(left) != unicode_latin_alpha(right))
        || (unicode_latin_digit(left) != unicode_latin_digit(right))
        || (unicode_whitespace(left) != unicode_whitespace(right))
        || is_punctuation(left)
        || is_punctuation(right)
}

/// Returns true if the sequence `<left><right>` represents a hard token boundary.
fn is_hard_boundary(left: char, right: char) -> bool {
    right == '\0' || left == '(' || left == ')' || right == '(' || right == ')'
}

fn is_unicode_hex_digit(c: char) -> bool {
    match c {
        '0'..='9' | 'a'..='f' | 'A'..='F' => true,
        _ => false,
    }
}

fn hex_to_char(hex: &str) -> Option<char> {
    let mut num = 0u32;
    for c in hex.chars() {
        num <<= 4;
        num += match c {
            '0'..='9' => c as u32 - '0' as u32,
            'a'..='f' => 10 + (c as u32 - 'a' as u32),
            'A'..='F' => 10 + (c as u32 - 'A' as u32),
            _ => return None,
        }
    }

    if let Ok(c) = char::try_from(num) {
        Some(c)
    } else {
        None
    }
}

/// Strips matching quote symbols from the beginning and end of the given string
/// (removing all quotes if given a single quote `'`)
pub(crate) fn dequote<'a, 'b>(s: &'a str, quotes: &'b str) -> &'a str {
    // note that this returns a new ref to the same string, rather
    // than modifying its argument as the C++ version does.
    if let Some(first_char) = s.chars().next() {
        if let Some(last_char) = s.chars().rev().next() {
            if first_char == last_char && quotes.contains(first_char) {
                let quote_len = first_char.len_utf8();
                if s.len() > 2 * quote_len {
                    return &s[quote_len..s.len() - quote_len];
                } else {
                    return "";
                }
            }
        }
    }
    s
}

pub(crate) fn read_word_quoted(text: &str, quotes: &str, cursor: usize) -> Option<(String, usize)> {
    let mut pos = cursor;
    let mut res = String::new();
    let mut skipchars = 0;

    let mut chars = text.get(cursor..)?.chars();
    let quote = chars.next();
    if quote.is_none() {
        return None;
    }
    let quote = quote.unwrap();
    if !quotes.contains(quote) {
        return None;
    }

    res.push(quote);
    pos += quote.len_utf8();

    for c in chars {
        if skipchars > 0 {
            skipchars -= 1;
            pos += c.len_utf8();
            continue;
        }
        if c == quote {
            res.push(c);
            pos += quote.len_utf8();
            return Some((res, pos));
        }

        if c == 'U' {
            if let Some('+') = text.get(pos + 1..).unwrap().chars().next() {
                if let Some(hex) = text.get(pos + 2..pos + 6) {
                    if let Some(c) = hex_to_char(hex) {
                        res.push(c);
                        skipchars += 5;
                    } else {
                        res.push('U');
                    }
                } else {
                    res.push('U');
                }
            } else {
                res.push('U');
            }
        } else if c == '\\' {
            match text.get(pos + 1..).unwrap().chars().next() {
                None => res.push(c),
                Some('b') => res.push('\x08'),
                Some('f') => res.push('\x0c'),
                Some('n') => res.push('\x0a'),
                Some('r') => res.push('\x0d'),
                Some('t') => res.push('\x09'),
                Some('v') => res.push('\x0b'),
                Some('u') => {
                    if let Some(hex) = text.get(pos + 2..pos + 6) {
                        if let Some(c) = hex_to_char(hex) {
                            res.push(c);
                            skipchars += 4;
                        } else {
                            res.push('u')
                        }
                    } else {
                        res.push('u')
                    }
                }
                Some(c @ _) => res.push(c),
            }
            skipchars += 1;
        } else {
            res.push(c);
        }

        pos += c.len_utf8();
    }

    None
}

pub(crate) fn read_word_unquoted(text: &str, cursor: usize) -> Option<(String, usize)> {
    let mut pos = cursor;
    let mut res = String::new();
    let mut prev = None;
    let mut skipchars = 0;

    for c in text.get(cursor..)?.chars() {
        if skipchars > 0 {
            skipchars -= 1;
            pos += c.len_utf8();
            prev = Some(c);
            continue;
        }
        if unicode_whitespace(c) {
            break;
        }
        if let Some(p) = prev {
            if is_hard_boundary(p, c) {
                break;
            }
        }

        if c == 'U' {
            if let Some('+') = text.get(pos + 1..).unwrap().chars().next() {
                if let Some(hex) = text.get(pos + 2..pos + 6) {
                    if let Some(c) = hex_to_char(hex) {
                        res.push(c);
                        skipchars += 5;
                    } else {
                        res.push('U');
                    }
                } else {
                    res.push('U');
                }
            } else {
                res.push('U');
            }
        } else if c == '\\' {
            match text.get(pos + 1..).unwrap().chars().next() {
                None => res.push(c),
                Some('b') => res.push('\x08'),
                Some('f') => res.push('\x0c'),
                Some('n') => res.push('\x0a'),
                Some('r') => res.push('\x0d'),
                Some('t') => res.push('\x09'),
                Some('v') => res.push('\x0b'),
                Some('u') => {
                    if let Some(hex) = text.get(pos + 2..pos + 6) {
                        if let Some(c) = hex_to_char(hex) {
                            res.push(c);
                            skipchars += 4;
                        } else {
                            res.push('u')
                        }
                    } else {
                        res.push('u')
                    }
                }
                Some(c @ _) => res.push(c),
            }
            skipchars += 1;
        } else {
            res.push(c);
        }

        pos += c.len_utf8();
        prev = Some(c);
    }

    if pos != cursor {
        Some((res, pos))
    } else {
        None
    }
}

fn common_length(s1: &str, s2: &str) -> usize {
    s1.chars()
        .zip(s2.chars())
        .take_while(|(c1, c2)| c1 == c2)
        .collect::<Vec<_>>()
        .len()
}

/// Returns true if the given string must have been shell-quoted
pub(crate) fn was_quoted(s: &str) -> bool {
    s.contains(&[' ', '\t', '(', ')', '<', '>', '&', '~'][..])
}

#[derive(Debug, PartialEq)]
pub(crate) struct DecomposedPair {
    pub(crate) name: String,
    pub(crate) modifier: String,
    pub(crate) separator: String,
    pub(crate) value: String,
}

/// Parse ("decompose") a pair into its constituent parts.  This assumes the text is a valid pair
/// string.
pub(crate) fn decompose_pair(text: &str) -> Option<DecomposedPair> {
    let npos = usize::max_value();
    let dot = text.find(".").unwrap_or(npos);
    let sep_defer = text.find("::").unwrap_or(npos);
    let sep_eval = text.find(":=").unwrap_or(npos);
    let sep_colon = text.find(":").unwrap_or(npos);
    let sep_equal = text.find("=").unwrap_or(npos);

    let (sep, sep_end) = if sep_defer != npos
        && sep_defer <= sep_eval
        && sep_defer <= sep_colon
        && sep_defer <= sep_equal
    {
        (sep_defer, sep_defer + 2)
    } else if sep_eval != npos
        && sep_eval <= sep_defer
        && sep_eval <= sep_colon
        && sep_eval <= sep_equal
    {
        (sep_eval, sep_eval + 2)
    } else if sep_colon != npos
        && sep_colon <= sep_defer
        && sep_colon <= sep_eval
        && sep_colon <= sep_equal
    {
        (sep_colon, sep_colon + 1)
    } else if sep_equal != npos
        && sep_equal <= sep_defer
        && sep_equal <= sep_eval
        && sep_equal <= sep_colon
    {
        (sep_equal, sep_equal + 1)
    } else {
        return None;
    };

    let (name, modifier) = if dot != npos && dot < sep {
        (
            text.get(0..dot).unwrap().into(),
            text.get(dot + 1..sep).unwrap().into(),
        )
    } else {
        (text.get(0..sep).unwrap().into(), "".into())
    };

    let separator = text.get(sep..sep_end).unwrap().into();
    let value = text.get(sep_end..).unwrap().into();

    Some(DecomposedPair {
        name,
        modifier,
        separator,
        value,
    })
}

#[derive(Debug, PartialEq)]
pub(crate) struct DecomposedSubstitution {
    pub(crate) from: String,
    pub(crate) to: String,
    pub(crate) flags: String,
}

/// Parse ("decompose") a substitution into its constituent parts.  This assumes
/// the text is a valid substitution string.
pub(crate) fn decompose_substitution(text: &str) -> Option<DecomposedSubstitution> {
    let mut cursor = 0;
    if let Some((from, from_curs)) = read_word_quoted(text, "/", cursor) {
        cursor = from_curs - 1;
        if let Some((to, to_curs)) = read_word_quoted(text, "/", cursor) {
            cursor = to_curs;
            let from = dequote(&from, "/").into();
            let to = dequote(&to, "/").into();
            let flags = text[cursor..].into();
            return Some(DecomposedSubstitution { from, to, flags });
        }
    }
    None
}

#[derive(Debug, PartialEq)]
pub(crate) struct DecomposedPattern {
    pub(crate) pattern: String,
    pub(crate) flags: String,
}

/// Parse ("decompose") a pattern into its constituent parts.  This assumes the text is a valid
/// pattern string.
pub(crate) fn decompose_pattern(text: &str) -> Option<DecomposedPattern> {
    let mut cursor = 0;
    if let Some((pattern, pattern_curs)) = read_word_quoted(text, "/", cursor) {
        cursor = pattern_curs;
        let pattern = dequote(&pattern, "/").into();
        let flags = text[cursor..].into();
        return Some(DecomposedPattern { pattern, flags });
    }
    None
}

impl Lexer {
    pub fn new<S: Into<String>>(text: S) -> Lexer {
        let text = text.into();
        let eos = text.len();
        Lexer {
            text,
            cursor: 0,
            eos,
            attributes: vec![],
        }
    }

    pub fn add_attribute<S: Into<String>>(&mut self, attribute: S) {
        self.attributes.push(attribute.into());
    }

    /// This static method tokenizes the input, but discards the type information.
    pub fn split<S: Into<String>>(text: S) -> Vec<String> {
        Lexer::new(text).into_iter().map(|(tx, ty)| tx).collect()
    }

    pub fn token(&mut self) -> Option<(String, Type)> {
        // Eat whitespace
        while let Some(c) = self.text[self.cursor..].chars().next() {
            if unicode_whitespace(c) {
                self.cursor += c.len_utf8();
                continue;
            }
            break;
        }

        if self.cursor == self.eos {
            return None;
        }

        // The sequence is specific, and must follow these rules:
        //   - date < duration < uuid < identifier
        //   - dom < uuid
        //   - uuid < hex < number
        //   - url < pair < identifier
        //   - hex < number
        //   - separator < tag < operator
        //   - path < substitution < pattern
        //   - set < number
        //   - word last
        if let Some(r) = self.is_string("\"'") {
            return Some(r);
        }
        if let Some(r) = self.is_date() {
            return Some(r);
        }
        if let Some(r) = self.is_duration() {
            return Some(r);
        }
        if let Some(r) = self.is_url() {
            return Some(r);
        }
        if let Some(r) = self.is_pair() {
            return Some(r);
        }
        if let Some(r) = self.is_uuid(true) {
            return Some(r);
        }
        if let Some(r) = self.is_set() {
            return Some(r);
        }
        if let Some(r) = self.is_dom() {
            return Some(r);
        }
        if let Some(r) = self.is_hexnumber() {
            return Some(r);
        }
        if let Some(r) = self.is_number() {
            return Some(r);
        }
        if let Some(r) = self.is_separator() {
            return Some(r);
        }
        if let Some(r) = self.is_tag() {
            return Some(r);
        }
        if let Some(r) = self.is_path() {
            return Some(r);
        }
        if let Some(r) = self.is_substitution() {
            return Some(r);
        }
        if let Some(r) = self.is_pattern() {
            return Some(r);
        }
        if let Some(r) = self.is_operator() {
            return Some(r);
        }
        if let Some(r) = self.is_identifier() {
            return Some(r);
        }
        if let Some(r) = self.is_word() {
            return Some(r);
        }
        None
    }

    pub fn is_eos(&self) -> bool {
        self.cursor == self.eos
    }

    // recognizers for the `token` method

    fn is_string(&mut self, quotes: &str) -> Option<(String, Type)> {
        if let Some((s, pos)) = read_word_quoted(&self.text, quotes, self.cursor) {
            self.cursor = pos;
            return Some((s, Type::String));
        }
        None
    }

    fn is_date(&mut self) -> Option<(String, Type)> {
        let (_, read) = DateTime::parse(&self.text[self.cursor..], "")?;
        let token = self.text[self.cursor..self.cursor + read].into();
        self.cursor += read;
        Some((token, Type::Date))
    }

    fn is_duration(&mut self) -> Option<(String, Type)> {
        let marker = self.cursor;

        if self.is_operator().is_some() {
            self.cursor = marker;
            return None;
        }

        let (_, read) = Duration::parse(&self.text[self.cursor..], "")?;
        let token = self.text[self.cursor..self.cursor + read].into();
        self.cursor += read;
        Some((token, Type::Duration))
    }

    fn is_url(&mut self) -> Option<(String, Type)> {
        let remainder = &self.text[self.cursor..];
        if remainder.starts_with("https://") || remainder.starts_with("http://") {
            if let Some(i) = remainder.find(unicode_whitespace) {
                let token = &remainder[..i];
                self.cursor += i;
                return Some((token.into(), Type::URL));
            } else {
                self.cursor = self.eos;
                return Some((remainder.into(), Type::URL));
            }
        }
        None
    }

    fn is_pair(&mut self) -> Option<(String, Type)> {
        let marker = self.cursor;
        if self.is_identifier().is_some() {
            let separator = &self.text[self.cursor..];
            if separator.starts_with("::") || separator.starts_with(":=") {
                self.cursor += 2;
            } else if separator.starts_with(":") || separator.starts_with("=") {
                self.cursor += 1;
            } else {
                self.cursor = marker;
                return None;
            }

            // String, word, or nothing are all valid
            let marker2 = self.cursor;
            if let Some((word, end)) = read_word_quoted(&self.text[..], "'\"", self.cursor) {
                self.cursor = end;
                return Some((
                    format!("{}{}", &self.text[marker..marker2], word),
                    Type::Pair,
                ));
            }
            if let Some((word, end)) = read_word_unquoted(&self.text[..], self.cursor) {
                self.cursor = end;
                return Some((
                    format!("{}{}", &self.text[marker..marker2], word),
                    Type::Pair,
                ));
            }
            if self.cursor == self.eos
                || unicode_whitespace(self.text[self.cursor..].chars().next().unwrap())
            {
                return Some((self.text[marker..self.cursor].into(), Type::Pair));
            }
        }
        self.cursor = marker;
        None
    }

    fn is_uuid(&mut self, end_boundary: bool) -> Option<(String, Type)> {
        let mut i = 0;
        for c in self.text[self.cursor..].chars() {
            if UUID_PATTERN[i] == b'x' {
                if !is_unicode_hex_digit(c) {
                    break;
                }
            } else {
                if c != '-' {
                    break;
                }
            }
            i += 1;
            if i >= UUID_PATTERN.len() {
                break;
            }
        }

        if i < UUID_MIN_LENGTH {
            return None;
        }

        if end_boundary {
            let c = self.text[self.cursor + i..].chars().next();
            if let Some(c) = c {
                if !unicode_whitespace(c) && !is_single_char_operator(c) {
                    return None;
                }
            }
        }

        let token = self.text[self.cursor..self.cursor + i].into();
        self.cursor += i;
        Some((token, Type::Uuid))
    }

    fn is_set(&mut self) -> Option<(String, Type)> {
        let marker = self.cursor;
        let mut count = 0;
        loop {
            if self.is_integer().is_some() {
                count += 1;
                if self.is_literal("-", false, false) {
                    if self.is_integer().is_some() {
                        count += 1;
                    } else {
                        self.cursor = marker;
                        return None;
                    }
                }
            } else {
                self.cursor = marker;
                return None;
            }
            if !self.is_literal(",", false, false) {
                break;
            }
        }

        if count <= 1 {
            self.cursor = marker;
            return None;
        }

        // -1 is OK here since integers are ASCII
        let last_char = self.text[self.cursor - 1..].chars().next().unwrap();

        // look ahead a bit
        match self.text[self.cursor..].chars().next() {
            Some(c) if !unicode_whitespace(c) && !is_hard_boundary(last_char, c) => {
                self.cursor = marker;
                return None;
            }
            _ => (),
        }

        Some((self.text[marker..self.cursor].into(), Type::Set))
    }

    fn is_dom(&mut self) -> Option<(String, Type)> {
        let marker = self.cursor;

        // rc. ...
        if self.is_literal("rc.", false, false) && self.is_word().is_some() {
            return Some((self.text[marker..self.cursor].into(), Type::DOM));
        } else {
            self.cursor = marker;
        }

        // Literals
        if self.is_one_of(
            &vec![
                "tw.syncneeded",
                "tw.program",
                "tw.args",
                "tw.width",
                "tw.height",
                "tw.version",
                "context.program",
                "context.args",
                "context.width",
                "context.height",
                "system.version",
                "system.os",
            ],
            false,
            true,
        ) {
            return Some((self.text[marker..self.cursor].into(), Type::DOM));
        }

        // Optional:
        //   <uuid>.
        //   <id>.
        if self.is_uuid(false).is_some() || self.is_integer().is_some() {
            if !self.is_literal(".", false, false) {
                self.cursor = marker;
                return None;
            }
        }

        // Any failure after this line should rollback to the checkpoint.
        let checkpoint = self.cursor;

        // [prefix]tags.<word>
        if self.is_literal("tags", false, false)
            && self.is_literal(".", false, false)
            && self.is_word().is_some()
        {
            return Some((self.text[marker..self.cursor].into(), Type::DOM));
        } else {
            self.cursor = checkpoint;
        }

        // [prefix]attribute (bounded)
        // (have to clone here to avoid double-borrowing self
        let attributes = self.attributes.clone();
        if self.is_one_of(&attributes, false, true) {
            return Some((self.text[marker..self.cursor].into(), Type::DOM));
        }

        // [prefix]attribute. (unbounded)
        if self.is_one_of(&attributes, false, false) {
            if self.is_literal(".", false, false) {
                let attribute = &self.text[checkpoint..self.cursor - 1];
                // if attribute type is 'date', then it has sub-elements.
                if attribute == "date" && self.is_one_of(&DATE_SUBELEMENTS, false, true) {
                    return Some((self.text[marker..self.cursor].into(), Type::DOM));
                }
                self.cursor = checkpoint;
            }
            // Lookahead: !<alpha>
            else if !self.text[marker..]
                .chars()
                .next()
                .map_or(false, |c| unicode_latin_alpha(c))
            {
                return Some((self.text[marker..self.cursor].into(), Type::DOM));
            }
            self.cursor = checkpoint;
        }

        // [prefix]annotations.
        if self.is_literal("annotations", true, false) && self.is_literal(".", false, false) {
            if self.is_literal("count", false, false) {
                return Some((self.text[marker..self.cursor].into(), Type::DOM));
            }

            if self.is_integer().is_some() {
                if self.is_literal(".", false, false) {
                    if self.is_literal("description", false, true) {
                        return Some((self.text[marker..self.cursor].into(), Type::DOM));
                    } else if self.is_literal("entry", false, true) {
                        return Some((self.text[marker..self.cursor].into(), Type::DOM));
                    } else if self.is_literal("entry", false, false)
                        && self.is_literal(".", false, false)
                        && self.is_one_of(&DATE_SUBELEMENTS, false, true)
                    {
                        return Some((self.text[marker..self.cursor].into(), Type::DOM));
                    }
                }
            } else {
                self.cursor = checkpoint;
            }
        }

        self.cursor = marker;
        None
    }

    fn is_hexnumber(&mut self) -> Option<(String, Type)> {
        let remainder = &self.text[self.cursor..];

        if !remainder.starts_with("0x") {
            return None;
        }
        let mut end = 2;
        for (i, c) in remainder[2..].char_indices() {
            if is_unicode_hex_digit(c) {
                end = 2 + i + c.len_utf8();
            } else {
                break;
            }
        }
        if end > 2 {
            self.cursor += end;
            Some((remainder[..end].into(), Type::Hex))
        } else {
            None
        }
    }

    fn is_number(&mut self) -> Option<(String, Type)> {
        let remainder = &self.text[self.cursor..];
        let mut chars = remainder.char_indices().peekable();
        let mut marker = 0;

        // A hand-rolled regexp.  States are as follows:
        //   \d \d* (. \d \d*)? ([eE] [+-]? \d \d* (.  \d \d*)?)?
        // 0 1  2    3 4  5      6    7     8  9    10 11 12
        let mut state = 0;

        loop {
            let c = match chars.peek() {
                Some((i, c)) => {
                    marker = *i;
                    Some(*c)
                }
                None => None,
            };
            match (state, c) {
                (0, Some(c)) if unicode_latin_digit(c) => state = 1,

                (1, Some(c)) if unicode_latin_digit(c) => state = 2,
                (1, Some(c)) if c == '.' => state = 3,
                (1, Some(c)) if c == 'e' || c == 'E' => state = 6,
                (1, _) => break,

                (2, Some(c)) if unicode_latin_digit(c) => state = 2,
                (2, Some(c)) if c == '.' => state = 3,
                (2, Some(c)) if c == 'e' || c == 'E' => state = 6,
                (2, _) => break,

                (3, Some(c)) if unicode_latin_digit(c) => state = 4,
                (3, Some(c)) if c == 'e' || c == 'E' => state = 6,
                (3, _) => break,

                (4, Some(c)) if unicode_latin_digit(c) => state = 5,
                (4, Some(c)) if c == 'e' || c == 'E' => state = 6,
                (4, _) => break,

                (5, Some(c)) if unicode_latin_digit(c) => state = 5,
                (5, Some(c)) if c == 'e' || c == 'E' => state = 6,
                (5, _) => break,

                (6, Some(c)) if unicode_latin_digit(c) => state = 8,
                (6, Some(c)) if c == '-' || c == '+' => state = 7,
                (6, _) => break,

                (7, Some(c)) if unicode_latin_digit(c) => state = 8,
                (7, _) => break,

                (8, Some(c)) if unicode_latin_digit(c) => state = 9,
                (8, Some(c)) if c == '.' => state = 10,
                (8, _) => break,

                (9, Some(c)) if unicode_latin_digit(c) => state = 9,
                (9, Some(c)) if c == '.' => state = 10,
                (9, _) => break,

                (10, Some(c)) if unicode_latin_digit(c) => state = 11,
                (10, _) => break,

                (11, Some(c)) if unicode_latin_digit(c) => state = 11,
                (11, _) => break,

                _ => return None,
            };
            if let Some((i, c)) = chars.next() {
                marker = i + c.len_utf8();
            }
        }
        // lookahead
        if let Some((_, c)) = chars.peek() {
            if !unicode_whitespace(*c) && !is_single_char_operator(*c) {
                return None;
            }
        }
        self.cursor += marker;
        Some((remainder[..marker].into(), Type::Number))
    }

    fn is_separator(&mut self) -> Option<(String, Type)> {
        let next_chars = self
            .text
            .get(self.cursor..self.cursor + 2)?
            .chars()
            .collect::<Vec<_>>();
        if &next_chars[..] == &['-', '-'] {
            self.cursor += 2;
            return Some(("--".into(), Type::Separator));
        }
        None
    }

    fn is_tag(&mut self) -> Option<(String, Type)> {
        let mut marker = self.cursor;

        // Lookbehind: Assert ^ or preceded by whitespace, (, or ).
        if marker > 0 {
            // if the previous byte is not a valid character, then it's
            // not ( or )
            if let Some(lookbehind) = self.text.get(self.cursor - 1..) {
                if let Some(c) = lookbehind.chars().next() {
                    if !unicode_whitespace(c) && c != '(' && c != ')' {
                        return None;
                    }
                }
            } else {
                return None;
            }
        }

        let mut chars = self.text[marker..].chars();
        if let Some(c) = chars.next() {
            if c == '+' || c == '-' {
                marker += c.len_utf8();
                if let Some(c) = chars.next() {
                    if is_identifier_start(c) {
                        marker += c.len_utf8();
                        while let Some(c) = chars.next() {
                            if !is_identifier_next(c) {
                                break;
                            }
                            marker += c.len_utf8();
                        }
                        let token = self.text[self.cursor..marker].into();
                        self.cursor = marker;
                        return Some((token, Type::Tag));
                    }
                }
            }
        }

        None
    }

    fn is_path(&mut self) -> Option<(String, Type)> {
        let mut marker = self.cursor;
        let mut slash_count = 0;
        let mut chars = self.text[self.cursor..].chars().peekable();

        loop {
            if let Some('/') = chars.next() {
                marker += 1;
                slash_count += 1;
            } else {
                break;
            }

            if let Some(c) = chars.next() {
                if !unicode_whitespace(c) && c != '/' {
                    marker += 1;
                    while let Some(c) = chars.peek() {
                        if !unicode_whitespace(*c) && *c != '/' {
                            marker += 1;
                            chars.next();
                        } else {
                            break;
                        }
                    }
                } else {
                    break;
                }
            } else {
                break;
            }
        }

        if marker > self.cursor && slash_count > 3 {
            let token = self.text[self.cursor..marker].into();
            self.cursor = marker;
            return Some((token, Type::Path));
        }

        None
    }

    fn is_substitution(&mut self) -> Option<(String, Type)> {
        let marker = self.cursor;

        if let Some((_, end)) = read_word_quoted(&self.text, "/", self.cursor) {
            // end-1 to step back over the middle `/`
            if let Some((_, end)) = read_word_quoted(&self.text, "/", end - 1) {
                let mut remainder = self.text[end..].chars();
                return match remainder.next() {
                    None => {
                        self.cursor = end;
                        Some((self.text[marker..self.cursor].into(), Type::Substitution))
                    }
                    Some('g') => match remainder.next() {
                        None => {
                            self.cursor = end + 1;
                            Some((self.text[marker..self.cursor].into(), Type::Substitution))
                        }
                        Some(c) if unicode_whitespace(c) => {
                            self.cursor = end + 1;
                            Some((self.text[marker..self.cursor].into(), Type::Substitution))
                        }
                        _ => None,
                    },
                    Some(c) if unicode_whitespace(c) => {
                        self.cursor = end;
                        Some((self.text[marker..self.cursor].into(), Type::Substitution))
                    }
                    _ => None,
                };
            }
        }

        None
    }

    fn is_pattern(&mut self) -> Option<(String, Type)> {
        let marker = self.cursor;
        if let Some((_, end)) = read_word_quoted(&self.text, "/", self.cursor) {
            if end == self.eos || unicode_whitespace(self.text[end..].chars().next().unwrap()) {
                self.cursor = end;
                return Some((self.text[marker..self.cursor].into(), Type::Pattern));
            }
        }
        None
    }

    fn is_operator(&mut self) -> Option<(String, Type)> {
        let remainder = &self.text[self.cursor..];

        // operators that do not require a boundary afterward
        for strop in &[
            // custom stuff
            "_hastag_", "_notag_", "_neg_", "_pos_",
            // triple-char
            "!==", // and, xor below
            // double-char
            "==", "!=", "<=", ">=", "||", "&&", "!~", // or below
            // single-char
            "+", "-", "*", "/", "(", ")", "<", ">", "^", "!", "%", "=", "~",
        ] {
            if remainder.starts_with(strop) {
                self.cursor += strop.len();
                return Some((remainder[..strop.len()].into(), Type::Op));
            }
        }

        // operators that require a boundary afterward
        for strop in &["and", "xor", "!==", "or"] {
            if remainder.starts_with(strop) {
                if self.cursor + strop.len() == self.eos
                    || is_boundary(
                        remainder[strop.len() - 1..].chars().next().unwrap(),
                        remainder[strop.len()..].chars().next().unwrap(),
                    )
                {
                    self.cursor += strop.len();
                    return Some((remainder[..strop.len()].into(), Type::Op));
                }
            }
        }
        None
    }

    fn is_identifier(&mut self) -> Option<(String, Type)> {
        let mut chars = self.text.get(self.cursor..)?.chars();
        let start = self.cursor;
        let mut len = 0;

        if let Some(c) = chars.next() {
            if is_identifier_start(c) {
                len += c.len_utf8();
                for c in chars {
                    if !is_identifier_next(c) {
                        break;
                    }
                    len += c.len_utf8();
                }
                self.cursor += len;
                return Some((self.text.get(start..self.cursor)?.into(), Type::Identifier));
            }
        }

        None
    }

    fn is_word(&mut self) -> Option<(String, Type)> {
        let mut marker = self.cursor;
        for c in self.text[self.cursor..].chars() {
            if unicode_whitespace(c) || is_single_char_operator(c) {
                break;
            }
            marker += c.len_utf8();
        }

        if marker > self.cursor {
            let token = self.text[self.cursor..marker].into();
            self.cursor = marker;
            return Some((token, Type::Word));
        }

        None
    }

    // utilities that may modify self

    fn is_one_of<S: AsRef<str>>(
        &mut self,
        options: &[S],
        allow_abbreviations: bool,
        end_boundary: bool,
    ) -> bool {
        for option in options {
            if self.is_literal(option.as_ref(), allow_abbreviations, end_boundary) {
                return true;
            }
        }
        false
    }

    fn is_literal(&mut self, literal: &str, allow_abbreviations: bool, end_boundary: bool) -> bool {
        // calculate the number of common characters between the literal and the string being
        // parsed
        let common = common_length(literal, &self.text[self.cursor..]);

        // Without abbreviations, common must equal literal length.
        if !allow_abbreviations && common < literal.len() {
            return false;
        }

        if allow_abbreviations && common < MINIMUM_MATCH_LEN {
            return false;
        }

        if end_boundary {
            let c = self.text[self.cursor + common..].chars().next();
            if let Some(c) = c {
                if !unicode_whitespace(c) && !is_single_char_operator(c) {
                    return false;
                }
            }
        }

        self.cursor += common;

        true
    }

    fn is_integer(&mut self) -> Option<(String, Type)> {
        let mut marker = self.cursor;
        for c in self.text[self.cursor..].chars() {
            if !unicode_latin_digit(c) {
                break;
            }
            marker += c.len_utf8();
        }

        if marker > self.cursor {
            let token = self.text[self.cursor..marker].into();
            self.cursor = marker;
            return Some((token, Type::Number));
        }

        None
    }
}

pub(crate) struct LexerIterator(Lexer);

impl Iterator for LexerIterator {
    type Item = (String, Type);

    fn next(&mut self) -> Option<Self::Item> {
        self.0.token()
    }
}

impl IntoIterator for Lexer {
    type Item = (String, Type);
    type IntoIter = LexerIterator;

    fn into_iter(self) -> Self::IntoIter {
        LexerIterator(self)
    }
}

#[cfg(test)]
mod test {
    use super::*;
    const NONE: Option<(String, Type)> = None;

    #[test]
    fn test_is_punctuation_comma() {
        assert!(is_punctuation(','));
    }

    #[test]
    fn test_is_punctuation_slash() {
        assert!(is_punctuation('/'));
    }

    #[test]
    fn test_is_punctuation_at() {
        assert!(!is_punctuation('@'));
    }

    #[test]
    fn test_is_punctuation_hash() {
        assert!(!is_punctuation('#'));
    }

    #[test]
    fn test_is_punctuation_dollar() {
        assert!(!is_punctuation('$'));
    }

    #[test]
    fn test_is_punctuation_underscore() {
        assert!(!is_punctuation('_'));
    }

    #[test]
    fn test_is_punctuation_space() {
        assert!(!is_punctuation(' '));
    }

    #[test]
    fn test_is_punctuation_a() {
        assert!(!is_punctuation('a'));
    }

    #[test]
    fn test_is_punctuation_9() {
        assert!(!is_punctuation('9'));
    }

    #[test]
    fn test_is_punctuation_latin() {
        assert!(!is_punctuation('é'));
    }

    #[test]
    fn test_is_punctuation_euro() {
        assert!(!is_punctuation('€'));
    }

    #[test]
    fn test_is_punctuation_smile() {
        assert!(!is_punctuation('☺'));
    }

    #[test]
    fn test_is_punctuation_numeric() {
        assert!(!is_punctuation('¾'));
    }

    #[test]
    fn test_is_boundary() {
        assert!(is_boundary(' ', 'a'));
        assert!(is_boundary('a', ' '));
        assert!(is_boundary(' ', '+'));
        assert!(is_boundary(' ', ','));
        assert!(!is_boundary('3', '4'));
        assert!(is_boundary('(', '('));
        assert!(!is_boundary('r', 'd'));
    }

    #[test]
    fn test_was_quoted() {
        assert!(!was_quoted(""));
        assert!(!was_quoted("foo"));
        assert!(was_quoted("a b"));
        assert!(was_quoted("(a)"));
    }

    #[test]
    fn test_dequote() {
        assert_eq!(dequote("foo", "'\""), "foo");
        assert_eq!(dequote("'foo'", "'\""), "foo");
        assert_eq!(dequote("\"foo\"", "'\""), "foo");
        assert_eq!(dequote("'o\\'clock'", "'\""), "o\\'clock");
        // single quote char
        assert_eq!(dequote("'", "'\""), "");
        // multibyte quote char
        assert_eq!(dequote("éo\\'clocké", "é"), "o\\'clock");
    }

    #[test]
    fn test_token_empty() {
        let mut l = Lexer::new("");
        assert_eq!(l.token(), NONE);
        assert!(l.is_eos());
    }

    #[test]
    fn test_token_tokens() {
        let mut l = Lexer::new(
            " one 'two \\'three\\''+456-(1.3*2 - 0x12) 1.2e-3.4    foo.bar and '\\u20ac'",
        );
        assert!(!l.is_eos());
        assert_eq!(l.token(), Some((String::from("one"), Type::Identifier)));
        assert_eq!(
            l.token(),
            Some((String::from("'two 'three''"), Type::String))
        );
        assert_eq!(l.token(), Some((String::from("+"), Type::Op)));
        assert_eq!(l.token(), Some((String::from("456"), Type::Number)));
        assert_eq!(l.token(), Some((String::from("-"), Type::Op)));
        assert_eq!(l.token(), Some((String::from("("), Type::Op)));
        assert_eq!(l.token(), Some((String::from("1.3"), Type::Number)));
        assert_eq!(l.token(), Some((String::from("*"), Type::Op)));
        assert_eq!(l.token(), Some((String::from("2"), Type::Number)));
        assert_eq!(l.token(), Some((String::from("-"), Type::Op)));
        assert_eq!(l.token(), Some((String::from("0x12"), Type::Hex)));
        assert_eq!(l.token(), Some((String::from(")"), Type::Op)));
        assert_eq!(l.token(), Some((String::from("1.2e-3.4"), Type::Number)));
        assert_eq!(l.token(), Some((String::from("foo.bar"), Type::Identifier)));
        assert_eq!(l.token(), Some((String::from("and"), Type::Op)));
        assert_eq!(l.token(), Some((String::from("'€'"), Type::String)));
        assert_eq!(l.token(), None);
        assert!(l.is_eos());
    }

    #[test]
    fn test_token_short_numbers() {
        let mut l = Lexer::new("1 12 123 1234 12345 123456 1234567 123.45e 12.34e+");
        assert_eq!(l.token(), Some((String::from("1"), Type::Number)));
        assert_eq!(l.token(), Some((String::from("12"), Type::Number)));
        assert_eq!(l.token(), Some((String::from("123"), Type::Number)));
        assert_eq!(l.token(), Some((String::from("1234"), Type::Number)));
        assert_eq!(l.token(), Some((String::from("12345"), Type::Number)));
        assert_eq!(l.token(), Some((String::from("123456"), Type::Number)));
        assert_eq!(l.token(), Some((String::from("1234567"), Type::Number)));
        assert_eq!(l.token(), Some((String::from("123.45e"), Type::Number)));
        assert_eq!(l.token(), Some((String::from("12.34e+"), Type::Number)));
        assert_eq!(l.token(), None);
    }

    #[test]
    fn test_read_word_quoted_simple() {
        assert_eq!(
            read_word_quoted("'one two'", "'\"", 0),
            Some((String::from("'one two'"), 9))
        );
    }

    #[test]
    fn test_read_word_quoted_unterminated() {
        assert_eq!(
            read_word_quoted("'one two", "'\"", 0),
            None as Option<(String, usize)>
        );
    }

    #[test]
    fn test_read_word_quoted_backslash_u() {
        assert_eq!(
            read_word_quoted("'pay \\u20a43'", "'\"", 0),
            Some((String::from("'pay ₤3'"), 13))
        );
    }

    #[test]
    fn test_read_word_quoted_u_plus() {
        assert_eq!(
            read_word_quoted("\"pay U+20AC5\"", "'\"", 0),
            Some((String::from("\"pay €5\""), 13))
        );
    }

    #[test]
    fn test_read_word_unquoted_simple() {
        assert_eq!(
            read_word_unquoted("input", 0),
            Some((String::from("input"), 5))
        );
    }

    #[test]
    fn test_read_word_unquoted_escaped_space() {
        assert_eq!(
            read_word_unquoted("one\\ two", 0),
            Some((String::from("one two"), 8))
        );
    }

    #[test]
    fn test_read_word_unquoted_escaped_quote() {
        assert_eq!(
            read_word_unquoted("one\\\"two", 0),
            Some((String::from("one\"two"), 8))
        );
    }

    #[test]
    fn test_read_word_unquoted_escaped_newline() {
        assert_eq!(
            read_word_unquoted("one\\ntwo", 0),
            Some((String::from("one\x0atwo"), 8))
        );
    }

    #[test]
    fn test_read_word_unquoted_escaped_backslash_u() {
        assert_eq!(
            read_word_unquoted("pay\\u20a43", 0),
            Some((String::from("pay₤3"), 10))
        );
    }

    #[test]
    fn test_read_word_unquoted_incomplete_escaped_backslash_u() {
        assert_eq!(
            read_word_unquoted("\\u203", 0),
            Some((String::from("u203"), 5))
        );
    }

    #[test]
    fn test_read_word_unquoted_nonhex_escaped_backslash_u() {
        assert_eq!(
            read_word_unquoted("\\u2fghk", 0),
            Some((String::from("u2fghk"), 7))
        );
    }

    #[test]
    fn test_read_word_unquoted_escaped_u_plus() {
        assert_eq!(
            read_word_unquoted("payU+20AC4", 0),
            Some((String::from("pay€4"), 10))
        );
    }

    #[test]
    fn test_read_word_unquoted_incomplete_u_plus() {
        assert_eq!(
            read_word_unquoted("U+20A", 0),
            Some((String::from("U+20A"), 5))
        );
    }

    #[test]
    fn test_read_word_trailing_whitespace() {
        assert_eq!(
            read_word_unquoted("one      ", 0),
            Some((String::from("one"), 3))
        );
    }

    #[test]
    fn test_read_word_unquoted_several_words() {
        let text = "one 'two' three\\ four";
        assert_eq!(read_word_unquoted(text, 0), Some((String::from("one"), 3)));
        assert_eq!(
            read_word_unquoted(text, 4),
            Some((String::from("'two'"), 9))
        );
        assert_eq!(
            read_word_unquoted(text, 10),
            Some((String::from("three four"), 21))
        );
    }

    #[test]
    fn test_common_length_empty() {
        assert_eq!(common_length("", ""), 0);
    }

    #[test]
    fn test_common_length_match_one() {
        assert_eq!(common_length("a", "a"), 1);
    }

    #[test]
    fn test_common_length_match_longer() {
        assert_eq!(common_length("abcde", "abcde"), 5);
    }

    #[test]
    fn test_common_length_match_s2_short() {
        assert_eq!(common_length("abc", ""), 0);
    }

    #[test]
    fn test_common_length_match_differ() {
        assert_eq!(common_length("abc", "def"), 0);
    }

    #[test]
    fn test_common_length_match_s2_prefix() {
        assert_eq!(common_length("foobar", "foo"), 3);
    }

    #[test]
    fn test_common_length_match_s1_prefix() {
        assert_eq!(common_length("foo", "foobar"), 3);
    }

    #[test]
    fn test_is_string() {
        let mut l = Lexer::new("'one'");
        assert_eq!(l.is_string("'\""), Some(("'one'".into(), Type::String)));
        assert_eq!(l.cursor, 5);
    }

    #[test]
    fn test_is_string_negative() {
        let mut l = Lexer::new("one");
        assert_eq!(l.is_string("'\""), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_string_empty() {
        let mut l = Lexer::new("''");
        assert_eq!(l.is_string("'\""), Some(("''".into(), Type::String)));
        assert_eq!(l.cursor, 2);
    }

    #[test]
    fn test_is_string_escape() {
        let mut l = Lexer::new("'one\ttwo'");
        assert_eq!(
            l.is_string("'\""),
            Some(("'one\ttwo'".into(), Type::String))
        );
        assert_eq!(l.cursor, 9);
    }

    #[test]
    fn test_is_date_year_eos() {
        let mut l = Lexer::new("2015");
        assert_eq!(l.is_date(), Some(("2015".into(), Type::Date)));
        assert_eq!(l.cursor, 4);
    }

    #[test]
    fn test_is_date_epoch() {
        let mut l = Lexer::new("315532800");
        assert_eq!(l.is_date(), Some(("315532800".into(), Type::Date)));
        assert_eq!(l.cursor, 9);
    }

    #[test]
    fn test_is_date_year_ws() {
        let mut l = Lexer::new("2015  ");
        assert_eq!(l.is_date(), Some(("2015".into(), Type::Date)));
        assert_eq!(l.cursor, 4);
    }

    #[test]
    fn test_is_date_year_ident() {
        let mut l = Lexer::new("2015abc");
        assert_eq!(l.is_date(), Some(("2015".into(), Type::Date)));
        assert_eq!(l.cursor, 4);
    }

    #[test]
    fn test_is_date_year_plus() {
        let mut l = Lexer::new("2015+");
        assert_eq!(l.is_date(), Some(("2015".into(), Type::Date)));
        assert_eq!(l.cursor, 4);
    }

    #[test]
    fn test_is_date_year_minus() {
        let mut l = Lexer::new("2015-xyz");
        assert_eq!(l.is_date(), Some(("2015-".into(), Type::Date)));
        assert_eq!(l.cursor, 5);
    }

    #[test]
    fn test_is_duration_1w() {
        let mut l = Lexer::new("1w");
        assert_eq!(l.is_duration(), Some(("1w".into(), Type::Duration)));
        assert_eq!(l.cursor, 2);
    }

    #[test]
    fn test_is_duration_op() {
        let mut l = Lexer::new("!!");
        assert_eq!(l.is_duration(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_number_digit() {
        let mut l = Lexer::new("3");
        assert_eq!(l.is_number(), Some(("3".into(), Type::Number)));
        assert_eq!(l.cursor, 1);
    }

    #[test]
    fn test_is_number_integer() {
        let mut l = Lexer::new("13");
        assert_eq!(l.is_number(), Some(("13".into(), Type::Number)));
        assert_eq!(l.cursor, 2);
    }

    #[test]
    fn test_is_number_trailing_minus() {
        let mut l = Lexer::new("13-");
        assert_eq!(l.is_number(), Some(("13".into(), Type::Number)));
        assert_eq!(l.cursor, 2);
    }

    #[test]
    fn test_is_number_decimal() {
        let mut l = Lexer::new("1.3");
        assert_eq!(l.is_number(), Some(("1.3".into(), Type::Number)));
        assert_eq!(l.cursor, 3);
    }

    #[test]
    fn test_is_number_multiple_decimal() {
        let mut l = Lexer::new("1.3.4");
        assert_eq!(l.is_number(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_number_decimal_no_digits() {
        let mut l = Lexer::new("1.");
        assert_eq!(l.is_number(), Some(("1.".into(), Type::Number)));
        assert_eq!(l.cursor, 2);
    }

    #[test]
    fn test_is_number_decimal_multi_digit() {
        let mut l = Lexer::new("12.32");
        assert_eq!(l.is_number(), Some(("12.32".into(), Type::Number)));
        assert_eq!(l.cursor, 5);
    }

    #[test]
    fn test_is_number_decimal_e_no_exponent() {
        let mut l = Lexer::new("12.32e");
        assert_eq!(l.is_number(), Some(("12.32e".into(), Type::Number)));
        assert_eq!(l.cursor, 6);
    }

    #[test]
    fn test_is_number_decimal_e_plus_no_exponent() {
        let mut l = Lexer::new("12.32e+");
        assert_eq!(l.is_number(), Some(("12.32e+".into(), Type::Number)));
        assert_eq!(l.cursor, 7);
    }

    #[test]
    fn test_is_number_decimal_e_integer_exponent() {
        let mut l = Lexer::new("12.32e-12");
        assert_eq!(l.is_number(), Some(("12.32e-12".into(), Type::Number)));
        assert_eq!(l.cursor, 9);
    }

    #[test]
    fn test_is_number_decimal_e_decimal_exponent() {
        let mut l = Lexer::new("12.32e12.34");
        assert_eq!(l.is_number(), Some(("12.32e12.34".into(), Type::Number)));
        assert_eq!(l.cursor, 11);
    }

    #[test]
    fn test_is_number_integer_invalid_lookahead() {
        let mut l = Lexer::new("13a");
        assert_eq!(l.is_number(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_set_singletons() {
        let mut l = Lexer::new("12,13");
        assert_eq!(l.is_set(), Some(("12,13".into(), Type::Set)));
        assert_eq!(l.cursor, 5);
    }

    #[test]
    fn test_is_set_ranges() {
        let mut l = Lexer::new("12-13,19-200");
        assert_eq!(l.is_set(), Some(("12-13,19-200".into(), Type::Set)));
        assert_eq!(l.cursor, 12);
    }

    #[test]
    fn test_is_set_double_comma() {
        let mut l = Lexer::new("12-13,,19-200");
        assert_eq!(l.is_set(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_set_trailing_comma() {
        let mut l = Lexer::new("12-13,");
        assert_eq!(l.is_set(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_set_trailing_ws() {
        let mut l = Lexer::new("12-13  ");
        assert_eq!(l.is_set(), Some(("12-13".into(), Type::Set)));
        assert_eq!(l.cursor, 5);
    }

    #[test]
    fn test_is_set_trailing_non_hard_boundary() {
        let mut l = Lexer::new("12-13abc");
        assert_eq!(l.is_set(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_separator() {
        let mut l = Lexer::new("  -- ");
        l.cursor = 2;
        assert_eq!(l.is_separator(), Some(("--".into(), Type::Separator)));
        assert_eq!(l.cursor, 4);
    }

    #[test]
    fn test_is_separator_negative() {
        let mut l = Lexer::new("- ");
        assert_eq!(l.is_separator(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_tag_plus() {
        let mut l = Lexer::new("+foo");
        assert_eq!(l.is_tag(), Some(("+foo".into(), Type::Tag)));
        assert_eq!(l.cursor, 4);
    }

    #[test]
    fn test_is_tag_not_after_whitespace() {
        let mut l = Lexer::new("x+y");
        l.cursor = 1;
        assert_eq!(l.is_tag(), NONE);
        assert_eq!(l.cursor, 1);
    }

    #[test]
    fn test_is_tag_after_whitespace() {
        let mut l = Lexer::new(" +y");
        l.cursor = 1;
        assert_eq!(l.is_tag(), Some(("+y".into(), Type::Tag)));
        assert_eq!(l.cursor, 3);
    }

    #[test]
    fn test_is_tag_after_lparen() {
        let mut l = Lexer::new("(+y");
        l.cursor = 1;
        assert_eq!(l.is_tag(), Some(("+y".into(), Type::Tag)));
        assert_eq!(l.cursor, 3);
    }

    #[test]
    fn test_is_tag_after_rparen() {
        let mut l = Lexer::new(")+y");
        l.cursor = 1;
        assert_eq!(l.is_tag(), Some(("+y".into(), Type::Tag)));
        assert_eq!(l.cursor, 3);
    }

    #[test]
    fn test_is_tag_after_multibyte_char() {
        let mut l = Lexer::new("€+y");
        l.cursor = 3;
        assert_eq!(l.is_tag(), NONE);
        assert_eq!(l.cursor, 3);
    }

    #[test]
    fn test_is_url_http() {
        let mut l = Lexer::new("http://foo.com/bar");
        assert_eq!(l.is_url(), Some(("http://foo.com/bar".into(), Type::URL)));
        assert_eq!(l.cursor, 18);
    }

    #[test]
    fn test_is_url_https() {
        let mut l = Lexer::new("https://foo.com/bar");
        assert_eq!(l.is_url(), Some(("https://foo.com/bar".into(), Type::URL)));
        assert_eq!(l.cursor, 19);
    }

    #[test]
    fn test_is_url_ws() {
        let mut l = Lexer::new("https://foo.com/bar  ");
        assert_eq!(l.is_url(), Some(("https://foo.com/bar".into(), Type::URL)));
        assert_eq!(l.cursor, 19);
    }

    #[test]
    fn test_is_url_with_ops() {
        let mut l = Lexer::new("https://foo.com/bar()+-~");
        assert_eq!(
            l.is_url(),
            Some(("https://foo.com/bar()+-~".into(), Type::URL))
        );
        assert_eq!(l.cursor, 24);
    }

    #[test]
    fn test_is_url_negative() {
        let mut l = Lexer::new("file://foo.com/bar");
        assert_eq!(l.is_url(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_pair_double_colon() {
        let mut l = Lexer::new("foo::bar  ");
        assert_eq!(l.is_pair(), Some(("foo::bar".into(), Type::Pair)));
        assert_eq!(l.cursor, 8);
    }

    #[test]
    fn test_is_pair_colon_eq() {
        let mut l = Lexer::new("foo:=bar  ");
        assert_eq!(l.is_pair(), Some(("foo:=bar".into(), Type::Pair)));
        assert_eq!(l.cursor, 8);
    }

    #[test]
    fn test_is_pair_colon() {
        let mut l = Lexer::new("foo:bar  ");
        assert_eq!(l.is_pair(), Some(("foo:bar".into(), Type::Pair)));
        assert_eq!(l.cursor, 7);
    }

    #[test]
    fn test_is_pair_equal() {
        let mut l = Lexer::new("foo=bar");
        assert_eq!(l.is_pair(), Some(("foo=bar".into(), Type::Pair)));
        assert_eq!(l.cursor, 7);
    }

    #[test]
    fn test_is_pair_quoted() {
        let mut l = Lexer::new("foo='abc def'");
        assert_eq!(l.is_pair(), Some(("foo='abc def'".into(), Type::Pair)));
        assert_eq!(l.cursor, 13);
    }

    #[test]
    fn test_is_pair_quoted_escapes() {
        let mut l = Lexer::new("foo='abc\\u20acdef'");
        assert_eq!(l.is_pair(), Some(("foo='abc€def'".into(), Type::Pair)));
        assert_eq!(l.cursor, 18);
    }

    #[test]
    fn test_is_uuid_long_eof() {
        let u = "ffffffff-ffff-ffff-ffff-ffffffffff";
        let mut l = Lexer::new(u);
        assert_eq!(l.is_uuid(true), Some((u.into(), Type::Uuid)));
        assert_eq!(l.cursor, 34);
    }

    #[test]
    fn test_is_uuid_long_ws() {
        let u = "ffffffff-ffff-ffff-ffff-ffffffffff  kjdf";
        let mut l = Lexer::new(u);
        assert_eq!(l.is_uuid(true), Some((u[..34].into(), Type::Uuid)));
        assert_eq!(l.cursor, 34);
    }

    #[test]
    fn test_is_uuid_long_op() {
        let u = "ffffffff-ffff-ffff-ffff-ffffffffff+";
        let mut l = Lexer::new(u);
        assert_eq!(l.is_uuid(true), Some((u[..34].into(), Type::Uuid)));
        assert_eq!(l.cursor, 34);
    }

    #[test]
    fn test_is_uuid_long_bad_boundary() {
        let u = "ffffffff-ffff-ffff-ffff-ffffffffff_";
        let mut l = Lexer::new(u);
        assert_eq!(l.is_uuid(true), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_uuid_long_bad_boundary_ignored() {
        let u = "ffffffff-ffff-ffff-ffff-ffffffffff_";
        let mut l = Lexer::new(u);
        assert_eq!(l.is_uuid(false), Some((u[..34].into(), Type::Uuid)));
        assert_eq!(l.cursor, 34);
    }

    #[test]
    fn test_is_uuid_too_short() {
        let u = "ffffff";
        let mut l = Lexer::new(u);
        assert_eq!(l.is_uuid(true), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_path_simple() {
        let mut l = Lexer::new("/path/to/a/file");
        assert_eq!(l.is_path(), Some(("/path/to/a/file".into(), Type::Path)));
        assert_eq!(l.cursor, 15);
    }

    #[test]
    fn test_is_path_too_short() {
        let mut l = Lexer::new("/a/file");
        assert_eq!(l.is_path(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_path_trailing_slash() {
        let mut l = Lexer::new("/path/to/a/dir/");
        assert_eq!(l.is_path(), Some(("/path/to/a/dir/".into(), Type::Path)));
        assert_eq!(l.cursor, 15);
    }

    #[test]
    fn test_is_path_double_slash() {
        let mut l = Lexer::new("/a//file");
        assert_eq!(l.is_path(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_path_no_initial_slash() {
        let mut l = Lexer::new("a/path/to/a/file");
        assert_eq!(l.is_path(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_substitution_simple() {
        let mut l = Lexer::new("/foo/bar/");
        assert_eq!(
            l.is_substitution(),
            Some(("/foo/bar/".into(), Type::Substitution))
        );
        assert_eq!(l.cursor, 9);
    }

    #[test]
    fn test_is_substitution_simple_ws() {
        let mut l = Lexer::new("/foo/bar/  ");
        assert_eq!(
            l.is_substitution(),
            Some(("/foo/bar/".into(), Type::Substitution))
        );
        assert_eq!(l.cursor, 9);
    }

    #[test]
    fn test_is_substitution_simple_g() {
        let mut l = Lexer::new("/foo/bar/g");
        assert_eq!(
            l.is_substitution(),
            Some(("/foo/bar/g".into(), Type::Substitution))
        );
        assert_eq!(l.cursor, 10);
    }

    #[test]
    fn test_is_substitution_simple_g_ws() {
        let mut l = Lexer::new("/foo/bar/g  ");
        assert_eq!(
            l.is_substitution(),
            Some(("/foo/bar/g".into(), Type::Substitution))
        );
        assert_eq!(l.cursor, 10);
    }

    #[test]
    fn test_is_substitution_simple_not_g() {
        let mut l = Lexer::new("/foo/bar/h");
        assert_eq!(l.is_substitution(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_substitution_simple_not_g_op() {
        let mut l = Lexer::new("/foo/bar/+");
        assert_eq!(l.is_substitution(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_substitution_simple_g_but_not_ws() {
        let mut l = Lexer::new("/foo/bar/ghi");
        assert_eq!(l.is_substitution(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_pattern_simple() {
        let mut l = Lexer::new("/foo/");
        assert_eq!(l.is_pattern(), Some(("/foo/".into(), Type::Pattern)));
        assert_eq!(l.cursor, 5);
    }

    #[test]
    fn test_is_pattern_escaped() {
        let mut l = Lexer::new("/f\\u20A4o/");
        assert_eq!(l.is_pattern(), Some(("/f\\u20A4o/".into(), Type::Pattern)));
        assert_eq!(l.cursor, 10);
    }

    #[test]
    fn test_is_pattern_simple_trailing_ws() {
        let mut l = Lexer::new("/foo/\n\t");
        assert_eq!(l.is_pattern(), Some(("/foo/".into(), Type::Pattern)));
        assert_eq!(l.cursor, 5);
    }

    #[test]
    fn test_is_operator_hastag() {
        let mut l = Lexer::new("_hastag_");
        assert_eq!(l.is_operator(), Some(("_hastag_".into(), Type::Op)));
    }

    #[test]
    fn test_is_operator_notag() {
        let mut l = Lexer::new("_notag_");
        assert_eq!(l.is_operator(), Some(("_notag_".into(), Type::Op)));
    }

    #[test]
    fn test_is_operator_neg() {
        let mut l = Lexer::new("_neg_");
        assert_eq!(l.is_operator(), Some(("_neg_".into(), Type::Op)));
    }

    #[test]
    fn test_is_operator_xor() {
        let mut l = Lexer::new("xor");
        assert_eq!(l.is_operator(), Some(("xor".into(), Type::Op)));
    }

    #[test]
    fn test_is_identifier_empty() {
        let mut l = Lexer::new("");
        assert_eq!(l.is_identifier(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_identifier_multibyte_nonpunct_first_char() {
        let mut l = Lexer::new("☺");
        assert_eq!(l.is_identifier(), Some(("☺".into(), Type::Identifier)));
        assert_eq!(l.cursor, 3);
    }

    #[test]
    fn test_is_identifier_bad_first_char() {
        let mut l = Lexer::new("1abc");
        assert_eq!(l.is_identifier(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_identifier_bad_next_char() {
        let mut l = Lexer::new("a:bc");
        assert_eq!(l.is_identifier(), Some(("a".into(), Type::Identifier)));
        assert_eq!(l.cursor, 1);
    }

    #[test]
    fn test_is_identifier_ok() {
        let mut l = Lexer::new("abc");
        assert_eq!(l.is_identifier(), Some(("abc".into(), Type::Identifier)));
        assert_eq!(l.cursor, 3);
    }

    #[test]
    fn test_is_word_no() {
        let mut l = Lexer::new("+");
        assert!(l.is_word().is_none());
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_word_pending() {
        let mut l = Lexer::new("foo.PENDING");
        l.cursor = 4;
        assert_eq!(l.is_word(), Some(("PENDING".into(), Type::Word)));
        assert_eq!(l.cursor, 11);
    }

    #[test]
    fn test_is_word_to_eof() {
        let mut l = Lexer::new("abc");
        assert_eq!(l.is_word(), Some(("abc".into(), Type::Word)));
        assert_eq!(l.cursor, 3);
    }

    #[test]
    fn test_is_word_nonzero_start() {
        let mut l = Lexer::new("--abc");
        l.cursor = 2;
        assert_eq!(l.is_word(), Some(("abc".into(), Type::Word)));
        assert_eq!(l.cursor, 5);
    }

    #[test]
    fn test_is_word_to_ws() {
        let mut l = Lexer::new("abc def");
        assert_eq!(l.is_word(), Some(("abc".into(), Type::Word)));
        assert_eq!(l.cursor, 3);
    }

    #[test]
    fn test_is_word_to_op() {
        let mut l = Lexer::new("abc*def");
        assert_eq!(l.is_word(), Some(("abc".into(), Type::Word)));
        assert_eq!(l.cursor, 3);
    }

    #[test]
    fn test_split_simple() {
        assert_eq!(
            Lexer::split(" ( A or B ) "),
            vec![
                String::from("("),
                String::from("A"),
                String::from("or"),
                String::from("B"),
                String::from(")"),
            ]
        );
    }

    #[test]
    fn test_split_confusing() {
        assert_eq!(
            Lexer::split("  +-* a+b 12.3e4 'c d'"),
            vec![
                String::from("+"),
                String::from("-"),
                String::from("*"),
                String::from("a"),
                String::from("+"),
                String::from("b"),
                String::from("12.3e4"),
                String::from("'c d'"),
            ]
        );
    }

    #[test]
    fn test_decompose_pair_combos() {
        let name = "name";
        for modifier in ["", "mod"].iter() {
            for separator in [":", "=", "::", ":="].iter() {
                for value in ["", "value", "a:b", "a::b", "a=b", "a:=b"].iter() {
                    let input = format!(
                        "{}{}{}{}{}",
                        name,
                        if modifier.len() > 0 { "." } else { "" },
                        modifier,
                        separator,
                        value
                    );
                    assert_eq!(
                        decompose_pair(&input),
                        Some(DecomposedPair {
                            name: name.into(),
                            modifier: String::from(*modifier),
                            separator: String::from(*separator),
                            value: String::from(*value),
                        })
                    );
                }
            }
        }
    }

    #[test]
    fn decompose_substitution_no_flags() {
        assert_eq!(
            decompose_substitution("/a/b/"),
            Some(DecomposedSubstitution {
                from: "a".into(),
                to: "b".into(),
                flags: "".into(),
            })
        );
    }

    #[test]
    fn decompose_substitution_flags() {
        assert_eq!(
            decompose_substitution("/a/b/g"),
            Some(DecomposedSubstitution {
                from: "a".into(),
                to: "b".into(),
                flags: "g".into(),
            })
        );
    }

    #[test]
    fn decompose_pattern_no_flags() {
        assert_eq!(
            decompose_pattern("/foober/"),
            Some(DecomposedPattern {
                pattern: "foober".into(),
                flags: "".into(),
            })
        );
    }

    #[test]
    fn decompose_pattern_flags() {
        assert_eq!(
            decompose_pattern("/foober/g"),
            Some(DecomposedPattern {
                pattern: "foober".into(),
                flags: "g".into(),
            })
        );
    }

    #[test]
    fn test_is_one_of() {
        let mut l = Lexer::new("Grumpy.");
        let dwarves = vec![
            "Sneezy", "Doc", "Bashful", "Grumpy", "Happy", "Sleepy", "Dopey",
        ];
        assert!(!l.is_one_of(&dwarves, false, true));
        assert_eq!(l.cursor, 0);
        assert!(l.is_one_of(&dwarves, false, false));
        assert_eq!(l.cursor, 6);
    }

    #[test]
    fn test_is_integer_negative() {
        let mut l = Lexer::new("one");
        assert_eq!(l.is_integer(), NONE);
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_integer_positive() {
        let mut l = Lexer::new("123");
        assert_eq!(l.is_integer(), Some(("123".into(), Type::Number)));
        assert_eq!(l.cursor, 3);
    }

    #[test]
    fn test_is_integer_trailing_dot() {
        let mut l = Lexer::new("123.foo");
        assert_eq!(l.is_integer(), Some(("123".into(), Type::Number)));
        assert_eq!(l.cursor, 3);
    }

    #[test]
    fn test_is_integer_not_at_start() {
        let mut l = Lexer::new("abc.123.foo");
        l.cursor = 4;
        assert_eq!(l.is_integer(), Some(("123".into(), Type::Number)));
        assert_eq!(l.cursor, 7);
    }

    #[test]
    fn test_is_literal_no_match() {
        let mut l = Lexer::new("one.two");
        assert!(!l.is_literal("zero", false, false));
        assert_eq!(l.cursor, 0);
    }

    #[test]
    fn test_is_literal_multi() {
        let mut l = Lexer::new("one.two");
        assert!(l.is_literal("one", false, false));
        assert_eq!(l.cursor, 3);
        assert!(l.is_literal(".", false, false));
        assert_eq!(l.cursor, 4);
        assert!(l.is_literal("two", false, true));
        assert_eq!(l.cursor, 7);
    }

    #[test]
    fn test_is_literal_abbrev() {
        let mut l = Lexer::new("wonder");
        assert!(!l.is_literal("wonderful", false, false));
        assert_eq!(l.cursor, 0);
        assert!(l.is_literal("wonderful", true, false));
        assert_eq!(l.cursor, 6);
    }

    mod integ {
        use super::super::*;

        fn lexer_test(input: &str, expected: Vec<(&str, Type)>) {
            // isolated case..
            let mut lexer = Lexer::new(input);
            lexer.add_attribute("due");
            lexer.add_attribute("tags");
            lexer.add_attribute("description");
            let got: Vec<_> = lexer.into_iter().collect();
            let got_strs: Vec<_> = got.iter().map(|(s, t)| (s.as_ref(), *t)).collect();
            assert_eq!(got_strs, expected);

            // embedded case..
            let mut lexer = Lexer::new(format!(" {} ", input));
            lexer.add_attribute("due");
            lexer.add_attribute("tags");
            lexer.add_attribute("description");
            let got: Vec<_> = lexer.into_iter().collect();
            let got_strs: Vec<_> = got.iter().map(|(s, t)| (s.as_ref(), *t)).collect();
            assert_eq!(got_strs, expected);
        }

        #[test]
        fn test_pattern_foo() {
            lexer_test("/foo/", vec![("/foo/", Type::Pattern)]);
        }

        #[test]
        fn test_pattern_escaped_slash() {
            lexer_test("/a\\/b/", vec![("/a\\/b/", Type::Pattern)]);
        }

        #[test]
        fn test_pattern_quote() {
            lexer_test("/'/", vec![("/'/", Type::Pattern)]);
        }

        // Substitution
        //
        #[test]
        fn test_subst_g() {
            lexer_test("/from/to/g", vec![("/from/to/g", Type::Substitution)]);
        }

        #[test]
        fn test_subst() {
            lexer_test("/from/to/", vec![("/from/to/", Type::Substitution)]);
        }

        // Tag
        //
        #[test]
        fn test_tag_simple() {
            lexer_test("+tag", vec![("+tag", Type::Tag)]);
        }

        #[test]
        fn test_tag_negative() {
            lexer_test("-tag", vec![("-tag", Type::Tag)]);
        }

        #[test]
        fn test_tag_at() {
            lexer_test("+@tag", vec![("+@tag", Type::Tag)]);
        }

        // Path
        //
        #[test]
        fn test_path() {
            lexer_test(
                "/long/path/to/file.txt",
                vec![("/long/path/to/file.txt", Type::Path)],
            );
        }

        #[test]
        fn test_path_dir() {
            lexer_test(
                "/long/path/to/dir/",
                vec![("/long/path/to/dir/", Type::Path)],
            );
        }

        // Word
        //
        #[test]
        fn test_1_foo_bar() {
            lexer_test("1.foo.bar", vec![("1.foo.bar", Type::Word)]);
        }

        // Identifier
        //
        #[test]
        fn test_foo() {
            lexer_test("foo", vec![("foo", Type::Identifier)]);
        }

        #[test]
        fn test_multibyte_ident() {
            lexer_test("Çirçös", vec![("Çirçös", Type::Identifier)]);
        }

        #[test]
        fn test_multibyte_nonpunctuation_single_char() {
            lexer_test("☺", vec![("☺", Type::Identifier)]);
        }

        #[test]
        fn test_name() {
            lexer_test("name", vec![("name", Type::Identifier)]);
        }

        #[test]
        fn test_f1() {
            lexer_test("f1", vec![("f1", Type::Identifier)]);
        }

        #[test]
        fn test_foo_dot_bar() {
            lexer_test("foo.bar", vec![("foo.bar", Type::Identifier)]);
        }

        #[test]
        fn test_long_with_underscore() {
            lexer_test(
                "a1a1a1a1_a1a1_a1a1_a1a1_a1a1a1a1a1a1",
                vec![("a1a1a1a1_a1a1_a1a1_a1a1_a1a1a1a1a1a1", Type::Identifier)],
            );
        }

        // Word that starts wih 'or', which is an operator, but should be ignored.
        //
        #[test]
        fn test_starts_with_or() {
            lexer_test("ordinary", vec![("ordinary", Type::Identifier)]);
        }

        // DOM
        //
        #[test]
        fn test_due() {
            lexer_test("due", vec![("due", Type::DOM)]);
        }

        #[test]
        fn test_123_tags() {
            lexer_test("123.tags", vec![("123.tags", Type::DOM)]);
        }

        #[test]
        fn test_123_tags_pending() {
            lexer_test("123.tags.PENDING", vec![("123.tags.PENDING", Type::DOM)]);
        }

        #[test]
        fn test_123_description() {
            lexer_test("123.description", vec![("123.description", Type::DOM)]);
        }

        #[test]
        fn test_123_annotations_count() {
            lexer_test(
                "123.annotations.count",
                vec![("123.annotations.count", Type::DOM)],
            );
        }

        #[test]
        fn test_123_annotations_1_description() {
            lexer_test(
                "123.annotations.1.description",
                vec![("123.annotations.1.description", Type::DOM)],
            );
        }

        #[test]
        fn test_123_annotations_1_entry() {
            lexer_test(
                "123.annotations.1.entry",
                vec![("123.annotations.1.entry", Type::DOM)],
            );
        }

        #[test]
        fn test_123_annotations_1_entry_year() {
            lexer_test(
                "123.annotations.1.entry.year",
                vec![("123.annotations.1.entry.year", Type::DOM)],
            );
        }

        #[test]
        fn test_uuid_due() {
            lexer_test(
                "a360fc44-315c-4366-b70c-ea7e7520b749.due",
                vec![("a360fc44-315c-4366-b70c-ea7e7520b749.due", Type::DOM)],
            );
        }

        #[test]
        fn test_numeric_uuid_due() {
            lexer_test(
                "12345678-1234-1234-1234-123456789012.due",
                vec![("12345678-1234-1234-1234-123456789012.due", Type::DOM)],
            );
        }

        #[test]
        fn test_system_os() {
            lexer_test("system.os", vec![("system.os", Type::DOM)]);
        }

        #[test]
        fn test_rc_foo() {
            lexer_test("rc.foo", vec![("rc.foo", Type::DOM)]);
        }

        // URL
        //
        #[test]
        fn test_lexer_31() {
            lexer_test(
                "http://example.com",
                vec![("http://example.com", Type::URL)],
            );
        }

        #[test]
        fn test_lexer_32() {
            lexer_test(
                "https://foo.example.com",
                vec![("https://foo.example.com", Type::URL)],
            );
        }

        // String
        //
        #[test]
        fn test_quoted_string() {
            lexer_test("'one two'", vec![("'one two'", Type::String)]);
        }

        #[test]
        fn test_double_quoted_string() {
            lexer_test("\"three\"", vec![("\"three\"", Type::String)]);
        }

        #[test]
        fn test_string_quoted_with_escapes() {
            lexer_test("'\\''", vec![("'''", Type::String)]);
        }

        #[test]
        fn test_string_quoted_quotes() {
            lexer_test("\"\\\"\"", vec![("\"\"\"", Type::String)]);
        }

        #[test]
        fn test_quoted_tabs() {
            lexer_test("\"\tfoo\t\"", vec![("\"\tfoo\t\"", Type::String)]);
        }

        #[test]
        fn test_multibyte_slash_u() {
            lexer_test("\"\\u20A43\"", vec![("\"₤3\"", Type::String)]);
        }

        #[test]
        fn test_multibyte_u_plus() {
            lexer_test("\"U+20AC4\"", vec![("\"€4\"", Type::String)]);
        }

        // Number
        //
        #[test]
        fn test_one() {
            lexer_test("1", vec![("1", Type::Number)]);
        }

        #[test]
        fn test_pi() {
            lexer_test("3.14", vec![("3.14", Type::Number)]);
        }

        #[test]
        fn test_avogadro() {
            lexer_test("6.02217e23", vec![("6.02217e23", Type::Number)]);
        }

        #[test]
        fn test_expo() {
            lexer_test("1.2e-3.4", vec![("1.2e-3.4", Type::Number)]);
        }

        #[test]
        fn test_hex() {
            lexer_test("0x2f", vec![("0x2f", Type::Hex)]);
        }

        // Set (1,2,4-7,9)
        //
        #[test]
        fn test_set_pair() {
            lexer_test("1,2", vec![("1,2", Type::Set)]);
        }

        #[test]
        fn test_set_range() {
            lexer_test("1-2", vec![("1-2", Type::Set)]);
        }

        #[test]
        fn test_set_range_pair() {
            lexer_test("1-2,4", vec![("1-2,4", Type::Set)]);
        }

        #[test]
        fn test_set_range_pair_ws() {
            lexer_test("1-2,4 ", vec![("1-2,4", Type::Set)]);
        }

        #[test]
        fn test_set_range_pair_paren() {
            lexer_test("1-2,4(", vec![("1-2,4", Type::Set), ("(", Type::Op)]);
        }

        #[test]
        fn test_ranges_and_singletons() {
            lexer_test("1-2,4,6-8", vec![("1-2,4,6-8", Type::Set)]);
        }

        #[test]
        fn test_set_more_ranges_and_singletons() {
            lexer_test("1-2,4,6-8,10-12", vec![("1-2,4,6-8,10-12", Type::Set)]);
        }

        // Pair
        //
        #[test]
        fn test_name_colon_value() {
            lexer_test("name:value", vec![("name:value", Type::Pair)]);
        }

        #[test]
        fn test_name_eq_value() {
            lexer_test("name=value", vec![("name=value", Type::Pair)]);
        }

        #[test]
        fn test_name_colon_eq_value() {
            lexer_test("name:=value", vec![("name:=value", Type::Pair)]);
        }

        #[test]
        fn test_name_dot_mod_colon_value() {
            lexer_test("name.mod:value", vec![("name.mod:value", Type::Pair)]);
        }

        #[test]
        fn test_name_dot_mod_eq_value() {
            lexer_test("name.mod=value", vec![("name.mod=value", Type::Pair)]);
        }

        #[test]
        fn test_name_colon() {
            lexer_test("name:", vec![("name:", Type::Pair)]);
        }

        #[test]
        fn test_name_eq() {
            lexer_test("name=", vec![("name=", Type::Pair)]);
        }

        #[test]
        fn test_name_dot_mod_colon() {
            lexer_test("name.mod:", vec![("name.mod:", Type::Pair)]);
        }

        #[test]
        fn test_name_dot_mod_equal() {
            lexer_test("name.mod=", vec![("name.mod=", Type::Pair)]);
        }

        #[test]
        fn test_pro_quoted() {
            lexer_test("pro:'P 1'", vec![("pro:'P 1'", Type::Pair)]);
        }

        #[test]
        fn test_rc_colon_x() {
            lexer_test("rc:x", vec![("rc:x", Type::Pair)]);
        }

        #[test]
        fn test_rc_dot_name_colon_value() {
            lexer_test("rc.name:value", vec![("rc.name:value", Type::Pair)]);
        }

        #[test]
        fn test_rc_dot_name_eq_value() {
            lexer_test("rc.name=value", vec![("rc.name=value", Type::Pair)]);
        }

        #[test]
        fn test_rc_dot_name_colon_eq_value() {
            lexer_test("rc.name:=value", vec![("rc.name:=value", Type::Pair)]);
        }

        #[test]
        fn test_due_colon_eq_quoted() {
            lexer_test("due:='eow - 2d'", vec![("due:='eow - 2d'", Type::Pair)]);
        }

        #[test]
        fn test_name_colon_quoted_with_newline() {
            lexer_test("name:'foo\nbar'", vec![("name:'foo\nbar'", Type::Pair)]);
        }

        // Operator - complete set
        //
        #[test]
        fn test_caret() {
            lexer_test("^", vec![("^", Type::Op)]);
        }

        #[test]
        fn test_bang() {
            lexer_test("!", vec![("!", Type::Op)]);
        }

        #[test]
        fn test_neg() {
            lexer_test("_neg_", vec![("_neg_", Type::Op)]);
        }

        #[test]
        fn test_pos() {
            lexer_test("_pos_", vec![("_pos_", Type::Op)]);
        }

        #[test]
        fn test_hastag() {
            lexer_test("_hastag_", vec![("_hastag_", Type::Op)]);
        }

        #[test]
        fn test_notag() {
            lexer_test("_notag_", vec![("_notag_", Type::Op)]);
        }

        #[test]
        fn test_star() {
            lexer_test("*", vec![("*", Type::Op)]);
        }

        #[test]
        fn test_slash() {
            lexer_test("/", vec![("/", Type::Op)]);
        }

        #[test]
        fn test_percent() {
            lexer_test("%", vec![("%", Type::Op)]);
        }

        #[test]
        fn test_plus() {
            lexer_test("+", vec![("+", Type::Op)]);
        }

        #[test]
        fn test_minus() {
            lexer_test("-", vec![("-", Type::Op)]);
        }

        #[test]
        fn test_leq() {
            lexer_test("<=", vec![("<=", Type::Op)]);
        }

        #[test]
        fn test_geq() {
            lexer_test(">=", vec![(">=", Type::Op)]);
        }

        #[test]
        fn test_gt() {
            lexer_test(">", vec![(">", Type::Op)]);
        }

        #[test]
        fn test_lt() {
            lexer_test("<", vec![("<", Type::Op)]);
        }

        #[test]
        fn test_eq() {
            lexer_test("=", vec![("=", Type::Op)]);
        }

        #[test]
        fn test_double_eq() {
            lexer_test("==", vec![("==", Type::Op)]);
        }

        #[test]
        fn test_not_eq() {
            lexer_test("!=", vec![("!=", Type::Op)]);
        }

        #[test]
        fn test_not_double_eq() {
            lexer_test("!==", vec![("!==", Type::Op)]);
        }

        #[test]
        fn test_tilde() {
            lexer_test("~", vec![("~", Type::Op)]);
        }

        #[test]
        fn test_not_tilde() {
            lexer_test("!~", vec![("!~", Type::Op)]);
        }

        #[test]
        fn test_and() {
            lexer_test("and", vec![("and", Type::Op)]);
        }

        #[test]
        fn test_or() {
            lexer_test("or", vec![("or", Type::Op)]);
        }

        #[test]
        fn test_xor() {
            lexer_test("xor", vec![("xor", Type::Op)]);
        }

        #[test]
        fn test_lparen() {
            lexer_test("(", vec![("(", Type::Op)]);
        }

        #[test]
        fn test_rparen() {
            lexer_test(")", vec![(")", Type::Op)]);
        }

        // UUID
        //
        #[test]
        fn test_uuid_ffs() {
            lexer_test(
                "ffffffff-ffff-ffff-ffff-ffffffffffff",
                vec![("ffffffff-ffff-ffff-ffff-ffffffffffff", Type::Uuid)],
            );
        }

        #[test]
        fn test_uuid_00s() {
            lexer_test(
                "00000000-0000-0000-0000-0000000",
                vec![("00000000-0000-0000-0000-0000000", Type::Uuid)],
            );
        }

        #[test]
        fn test_uuid_shorter() {
            lexer_test(
                "00000000-0000-0000-0000",
                vec![("00000000-0000-0000-0000", Type::Uuid)],
            );
        }

        #[test]
        fn test_uuid_shorter_still() {
            lexer_test(
                "00000000-0000-0000",
                vec![("00000000-0000-0000", Type::Uuid)],
            );
        }

        #[test]
        fn test_uuid_even_shorter() {
            lexer_test("00000000-0000", vec![("00000000-0000", Type::Uuid)]);
        }

        #[test]
        fn test_uuid_only_first_bit() {
            lexer_test("00000000", vec![("00000000", Type::Uuid)]);
        }

        #[test]
        fn test_real_uuid() {
            lexer_test(
                "a360fc44-315c-4366-b70c-ea7e7520b749",
                vec![("a360fc44-315c-4366-b70c-ea7e7520b749", Type::Uuid)],
            );
        }

        #[test]
        fn test_real_uuid_shorter() {
            lexer_test(
                "a360fc44-315c-4366-b70c-ea7e752",
                vec![("a360fc44-315c-4366-b70c-ea7e752", Type::Uuid)],
            );
        }

        #[test]
        fn test_real_uuid_shorter_still() {
            lexer_test(
                "a360fc44-315c-4366-b70c",
                vec![("a360fc44-315c-4366-b70c", Type::Uuid)],
            );
        }

        #[test]
        fn test_real_uuid_even_shorter() {
            lexer_test(
                "a360fc44-315c-4366",
                vec![("a360fc44-315c-4366", Type::Uuid)],
            );
        }

        #[test]
        fn test_real_uuid_naming_is_hard() {
            lexer_test("a360fc44-315c", vec![("a360fc44-315c", Type::Uuid)]);
        }

        #[test]
        fn test_real_uuid_only_first_bit() {
            lexer_test("a360fc44", vec![("a360fc44", Type::Uuid)]);
        }

        // Date
        //
        #[test]
        fn test_year_week() {
            lexer_test("2015-W01", vec![("2015-W01", Type::Date)]);
        }

        #[test]
        fn test_year_month_day() {
            lexer_test("2015-02-17", vec![("2015-02-17", Type::Date)]);
        }

        #[test]
        fn test_timestamp() {
            lexer_test(
                "2013-11-29T22:58:00Z",
                vec![("2013-11-29T22:58:00Z", Type::Date)],
            );
        }

        #[test]
        fn test_abbrev_timestamp() {
            lexer_test("20131129T225800Z", vec![("20131129T225800Z", Type::Date)]);
        }

        #[test]
        fn test_9thn() {
            lexer_test("9th", vec![("9th", Type::Date)]);
        }

        #[test]
        fn test_10th() {
            lexer_test("10th", vec![("10th", Type::Date)]);
        }

        #[test]
        fn test_today() {
            lexer_test("today", vec![("today", Type::Date)]);
        }

        // Duration
        //
        #[test]
        fn test_year() {
            lexer_test("year", vec![("year", Type::Duration)]);
        }

        #[test]
        fn test_4weeks() {
            lexer_test("4weeks", vec![("4weeks", Type::Duration)]);
        }

        #[test]
        fn test_pt23h() {
            lexer_test("PT23H", vec![("PT23H", Type::Duration)]);
        }

        #[test]
        fn test_1second() {
            lexer_test("1second", vec![("1second", Type::Duration)]);
        }

        #[test]
        fn test_1s() {
            lexer_test("1s", vec![("1s", Type::Duration)]);
        }

        #[test]
        fn test_1minute() {
            lexer_test("1minute", vec![("1minute", Type::Duration)]);
        }

        #[test]
        fn test_2hour() {
            lexer_test("2hour", vec![("2hour", Type::Duration)]);
        }

        #[test]
        fn test_3_days() {
            lexer_test("3 days", vec![("3 days", Type::Duration)]);
        }

        #[test]
        fn test_4w() {
            lexer_test("4w", vec![("4w", Type::Duration)]);
        }

        #[test]
        fn test_5mo() {
            lexer_test("5mo", vec![("5mo", Type::Duration)]);
        }

        #[test]
        fn test_6_years() {
            lexer_test("6 years", vec![("6 years", Type::Duration)]);
        }

        #[test]
        fn test_p1y() {
            lexer_test("P1Y", vec![("P1Y", Type::Duration)]);
        }

        #[test]
        fn test_pt1h() {
            lexer_test("PT1H", vec![("PT1H", Type::Duration)]);
        }

        #[test]
        fn test_p_full() {
            lexer_test("P1Y1M1DT1H1M1S", vec![("P1Y1M1DT1H1M1S", Type::Duration)]);
        }

        // Misc
        //
        #[test]
        fn test_separator() {
            lexer_test("--", vec![("--", Type::Separator)]);
        }

        #[test]
        fn test_separator_ws() {
            lexer_test("  --  ", vec![("--", Type::Separator)]);
        }

        #[test]
        fn test_separator_boundaries() {
            lexer_test(
                "123--123  ",
                vec![
                    ("123", Type::Number),
                    ("--", Type::Separator),
                    ("123", Type::Number),
                ],
            );
        }

        // Expression
        // due:eom-2w
        // due < eom + 1w + 1d
        // ( /pattern/ or 8ad2e3db-914d-4832-b0e6-72fa04f6e331,3b6218f9-726a-44fc-aa63-889ff52be442 )
        //
        #[test]
        fn test_expression() {
            lexer_test(
                "(1+2)",
                vec![
                    ("(", Type::Op),
                    ("1", Type::Number),
                    ("+", Type::Op),
                    ("2", Type::Number),
                    (")", Type::Op),
                ],
            );
        }

        #[test]
        fn test_expression_dom_tilde() {
            lexer_test(
                "description~pattern",
                vec![
                    ("description", Type::DOM),
                    ("~", Type::Op),
                    ("pattern", Type::Identifier),
                ],
            );
        }

        #[test]
        fn test_expression_paren_tag() {
            lexer_test(
                "(+tag)",
                vec![("(", Type::Op), ("+tag", Type::Tag), (")", Type::Op)],
            );
        }

        #[test]
        fn test_expression_paren_name_value() {
            lexer_test(
                "(name:value)",
                vec![("(", Type::Op), ("name:value", Type::Pair), (")", Type::Op)],
            );
        }
    }
}
