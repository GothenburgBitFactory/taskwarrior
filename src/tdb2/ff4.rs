use std::str;
use std::io::{Result, Error, ErrorKind};

use super::pig::Pig;
use task::{TaskBuilder, Task};

/// Rust implementation of part of utf8_codepoint from Taskwarrior's src/utf8.cpp
///
/// Note that the original function will return garbage for invalid hex sequences;
/// this panics instead.
fn hex_to_unicode(value: &[u8]) -> String {
    if value.len() < 4 {
        panic!(format!("unicode escape too short -- {:?}", value));
    }

    fn nyb(c: u8) -> u16 {
        match c {
            b'0'...b'9' => (c - b'0') as u16,
            b'a'...b'f' => (c - b'a' + 10) as u16,
            b'A'...b'F' => (c - b'A' + 10) as u16,
            _ => panic!(format!("invalid hex character {:?}", c)),
        }
    };

    let words = [
        nyb(value[0]) << 12 | nyb(value[1]) << 8 | nyb(value[2]) << 4 | nyb(value[3]),
    ];
    return String::from_utf16(&words[..]).unwrap();
}

/// Rust implementation of JSON::decode in Taskwarrior's src/JSON.cpp
///
/// Decode the given byte slice into a string using Taskwarrior JSON's escaping The slice is
/// assumed to be ASCII; unicode escapes within it will be expanded.
fn json_decode(value: &[u8]) -> String {
    let length = value.len();
    let mut rv = String::with_capacity(length);

    let mut pos = 0;
    while pos < length {
        let v = value[pos];
        if v == b'\\' {
            pos += 1;
            if pos == length {
                rv.push(v as char);
                break;
            }
            let v = value[pos];
            match v {
                b'"' | b'\\' | b'/' => rv.push(v as char),
                b'b' => rv.push(8 as char),
                b'f' => rv.push(12 as char),
                b'n' => rv.push('\n' as char),
                b'r' => rv.push('\r' as char),
                b't' => rv.push('\t' as char),
                b'u' => {
                    rv.push_str(&hex_to_unicode(&value[pos + 1..]));
                    pos += 4;
                }
                _ => {
                    rv.push(b'\\' as char);
                    rv.push(v as char);
                }
            }
        } else {
            rv.push(v as char)
        }
        pos += 1;
    }

    rv
}

/// Rust implementation of Task::decode in Taskwarrior's src/Task.cpp
///
/// Note that the docstring for the C++ function does not match the
/// implementation!
fn decode(value: String) -> String {
    if let Some(_) = value.find('&') {
        return value.replace("&open;", "[").replace("&close;", "]");
    }
    value
}

/// Parse an "FF4" formatted task line.  From Task::parse in Taskwarrior's src/Task.cpp.
///
/// While Taskwarrior supports additional formats, this is the only format supported by rask.
pub(super) fn parse_ff4(line: &str) -> Result<Task> {
    let mut pig = Pig::new(line.as_bytes());
    let mut builder = TaskBuilder::new();

    if !pig.skip(b'[') {
        return Err(Error::new(ErrorKind::Other, "bad line"));
    }
    if let Some(line) = pig.get_until(b']') {
        let mut pig = Pig::new(line);
        while !pig.depleted() {
            if let Some(name) = pig.get_until(b':') {
                let name = str::from_utf8(name).unwrap();
                if !pig.skip(b':') {
                    return Err(Error::new(ErrorKind::Other, "bad line"));
                }
                if let Some(value) = pig.get_quoted(b'"') {
                    let value = json_decode(value);
                    let value = decode(value);
                    builder = builder.set(name, value);
                } else {
                    return Err(Error::new(ErrorKind::Other, "bad line"));
                }
                pig.skip(b' ');
            } else {
                return Err(Error::new(ErrorKind::Other, "bad line"));
            }
        }
    } else {
        return Err(Error::new(ErrorKind::Other, "bad line"));
    }
    if !pig.skip(b']') {
        return Err(Error::new(ErrorKind::Other, "bad line"));
    }
    if !pig.depleted() {
        return Err(Error::new(ErrorKind::Other, "bad line"));
    }
    Ok(builder.finish())
}

#[cfg(test)]
mod test {
    use super::{decode, json_decode, hex_to_unicode, parse_ff4};
    use task::Pending;

    #[test]
    fn test_hex_to_unicode_digits() {
        assert_eq!(hex_to_unicode(b"1234"), "\u{1234}");
    }

    #[test]
    fn test_hex_to_unicode_lower() {
        assert_eq!(hex_to_unicode(b"abcd"), "\u{abcd}");
    }

    #[test]
    fn test_hex_to_unicode_upper() {
        assert_eq!(hex_to_unicode(b"ABCD"), "\u{abcd}");
    }

    #[test]
    fn test_json_decode_no_change() {
        assert_eq!(json_decode(b"abcd"), "abcd");
    }

    #[test]
    fn test_json_decode_escape_quote() {
        assert_eq!(json_decode(b"ab\\\"cd"), "ab\"cd");
    }

    #[test]
    fn test_json_decode_escape_backslash() {
        assert_eq!(json_decode(b"ab\\\\cd"), "ab\\cd");
    }

    #[test]
    fn test_json_decode_escape_frontslash() {
        assert_eq!(json_decode(b"ab\\/cd"), "ab/cd");
    }

    #[test]
    fn test_json_decode_escape_b() {
        assert_eq!(json_decode(b"ab\\bcd"), "ab\x08cd");
    }

    #[test]
    fn test_json_decode_escape_f() {
        assert_eq!(json_decode(b"ab\\fcd"), "ab\x0ccd");
    }

    #[test]
    fn test_json_decode_escape_n() {
        assert_eq!(json_decode(b"ab\\ncd"), "ab\ncd");
    }

    #[test]
    fn test_json_decode_escape_r() {
        assert_eq!(json_decode(b"ab\\rcd"), "ab\rcd");
    }

    #[test]
    fn test_json_decode_escape_t() {
        assert_eq!(json_decode(b"ab\\tcd"), "ab\tcd");
    }

    #[test]
    fn test_json_decode_escape_other() {
        assert_eq!(json_decode(b"ab\\xcd"), "ab\\xcd");
    }

    #[test]
    fn test_json_decode_escape_eos() {
        assert_eq!(json_decode(b"ab\\"), "ab\\");
    }

    #[test]
    fn test_json_decode_escape_unicode() {
        assert_eq!(json_decode(b"ab\\u1234"), "ab\u{1234}");
    }

    #[test]
    fn test_decode_no_change() {
        let s = "abcd &quot; efgh &".to_string();
        assert_eq!(decode(s.clone()), s);
    }

    #[test]
    fn test_decode_multi() {
        let s = "abcd &open; efgh &close; &open".to_string();
        assert_eq!(decode(s), "abcd [ efgh ] &open".to_string());
    }

    #[test]
    fn test_parse_ff4() {
        let s = "[description:\"desc\" entry:\"1437855511\" modified:\"1479480556\" \
                 priority:\"L\" project:\"lists\" status:\"pending\" tags:\"watch\" \
                 uuid:\"83ce989e-8634-4d62-841c-eb309383ff1f\"]";
        let task = parse_ff4(s).unwrap();
        assert_eq!(task.status, Pending);
        assert_eq!(task.description, "desc");
    }
}
