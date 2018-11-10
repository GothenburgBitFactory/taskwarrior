mod nibbler;

use std::str;

use std::io::{stdin, BufRead, Result, Error, ErrorKind};
use nibbler::Nibbler;

/// Decode the given byte slice into a string using Taskwarrior JSON's escaping The slice is
/// assumed to be ASCII; unicode escapes within it will be expanded.
// TODO: return Cow
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
                b'u' => panic!("omg please no"),
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

fn decode(value: String) -> String {
    if let Some(_) = value.find('&') {
        return value.replace("&open;", "[").replace("&close;", "]");
    }
    value
}

fn parse_ff4(line: &str) -> Result<()> {
    let mut nib = Nibbler::new(line.as_bytes());
    println!("{}", line);

    if !nib.skip(b'[') {
        return Err(Error::new(ErrorKind::Other, "bad line"));
    }
    if let Some(line) = nib.get_until(b']') {
        let mut nib = Nibbler::new(line);
        while !nib.depleted() {
            if let Some(name) = nib.get_until(b':') {
                if !nib.skip(b':') {
                    return Err(Error::new(ErrorKind::Other, "bad line"));
                }
                if let Some(value) = nib.get_quoted(b'"') {
                    let value = json_decode(value);
                    let value = decode(value);
                    println!("{}={}", str::from_utf8(name).unwrap(), value);
                } else {
                    return Err(Error::new(ErrorKind::Other, "bad line"));
                }
                nib.skip(b' ');
            } else {
                return Err(Error::new(ErrorKind::Other, "bad line"));
            }
        }
    } else {
        return Err(Error::new(ErrorKind::Other, "bad line"));
    }
    if !nib.skip(b']') {
        return Err(Error::new(ErrorKind::Other, "bad line"));
    }
    if !nib.depleted() {
        return Err(Error::new(ErrorKind::Other, "bad line"));
    }
    Ok(())
}

fn parse_tdb2() -> Result<()> {
    let input = stdin();
    for line in input.lock().lines() {
        parse_ff4(&line?)?;
    }
    Ok(())
}

fn main() {
    parse_tdb2().unwrap();
    println!("Done");
}
