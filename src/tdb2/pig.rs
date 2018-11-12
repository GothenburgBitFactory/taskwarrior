//! A minimal implementation of the "Pig" parsing utility from the Taskwarrior
//! source.  This is just enough to parse FF4 lines.

use errors::*;

pub struct Pig<'a> {
    input: &'a [u8],
    cursor: usize,
}

impl<'a> Pig<'a> {
    pub fn new(input: &'a [u8]) -> Self {
        Pig {
            input: input,
            cursor: 0,
        }
    }

    pub fn get_until(&mut self, c: u8) -> Result<&'a [u8]> {
        if self.cursor >= self.input.len() {
            return Err(Error::from("input truncated"));
        }

        let mut i = self.cursor;
        while i < self.input.len() {
            if self.input[i] == c {
                let rv = &self.input[self.cursor..i];
                self.cursor = i;
                return Ok(rv);
            }
            i += 1;
        }
        let rv = &self.input[self.cursor..];
        self.cursor = self.input.len();
        Ok(rv)
    }

    pub fn get_quoted(&mut self, c: u8) -> Result<&'a [u8]> {
        let length = self.input.len();
        if self.cursor >= length || self.input[self.cursor] != c {
            return Err(Error::from(
                "quoted string does not begin with quote character",
            ));
        }

        let start = self.cursor + 1;
        let mut i = start;

        while i < length {
            while i < length && self.input[i] != c {
                i += 1
            }
            if i == length {
                return Err(Error::from("unclosed quote"));
            }
            if i == start {
                return Ok(&self.input[i..i]);
            }

            if self.input[i - 1] == b'\\' {
                // work backward looking for escaped backslashes
                let mut j = i - 2;
                let mut quote = true;
                while j >= start && self.input[j] == b'\\' {
                    quote = !quote;
                    j -= 1;
                }

                if quote {
                    i += 1;
                    continue;
                }
            }

            // none of the above matched, so we are at the end
            self.cursor = i + 1;
            return Ok(&self.input[start..i]);
        }

        unreachable!();
    }

    pub fn skip(&mut self, c: u8) -> Result<()> {
        if self.cursor < self.input.len() && self.input[self.cursor] == c {
            self.cursor += 1;
            return Ok(());
        }
        bail!(format!(
            "expected character `{}`",
            String::from_utf8(vec![c])?
        ));
    }

    pub fn depleted(&self) -> bool {
        self.cursor >= self.input.len()
    }
}

#[cfg(test)]
mod test {
    use super::Pig;

    #[test]
    fn test_get_until() {
        let s = b"abc:123";
        let mut pig = Pig::new(s);
        assert_eq!(pig.get_until(b':').unwrap(), &s[..3]);
    }

    #[test]
    fn test_get_until_empty() {
        let s = b"abc:123";
        let mut pig = Pig::new(s);
        assert_eq!(pig.get_until(b'a').unwrap(), &s[..0]);
    }

    #[test]
    fn test_get_until_not_found() {
        let s = b"abc:123";
        let mut pig = Pig::new(s);
        assert_eq!(pig.get_until(b'/').unwrap(), &s[..]);
    }

    #[test]
    fn test_get_quoted() {
        let s = b"'abcd'efg";
        let mut pig = Pig::new(s);
        assert_eq!(pig.get_quoted(b'\'').unwrap(), &s[1..5]);
        assert_eq!(pig.cursor, 6);
    }

    #[test]
    fn test_get_quoted_unopened() {
        let s = b"abcd'efg";
        let mut pig = Pig::new(s);
        assert!(pig.get_quoted(b'\'').is_err());
        assert_eq!(pig.cursor, 0); // nothing consumed
    }

    #[test]
    fn test_get_quoted_unclosed() {
        let s = b"'abcdefg";
        let mut pig = Pig::new(s);
        assert!(pig.get_quoted(b'\'').is_err());
        assert_eq!(pig.cursor, 0);
    }

    #[test]
    fn test_get_quoted_escaped() {
        let s = b"'abc\\'de'fg";
        let mut pig = Pig::new(s);
        assert_eq!(pig.get_quoted(b'\'').unwrap(), &s[1..8]);
        assert_eq!(pig.cursor, 9);
    }

    #[test]
    fn test_get_quoted_double_escaped() {
        let s = b"'abc\\\\'de'fg";
        let mut pig = Pig::new(s);
        assert_eq!(pig.get_quoted(b'\'').unwrap(), &s[1..6]);
        assert_eq!(pig.cursor, 7);
    }

    #[test]
    fn test_get_quoted_triple_escaped() {
        let s = b"'abc\\\\\\'de'fg";
        let mut pig = Pig::new(s);
        assert_eq!(pig.get_quoted(b'\'').unwrap(), &s[1..10]);
        assert_eq!(pig.cursor, 11);
    }

    #[test]
    fn test_get_quoted_all_escapes() {
        let s = b"'\\\\\\'\\\\'fg";
        let mut pig = Pig::new(s);
        assert_eq!(pig.get_quoted(b'\'').unwrap(), &s[1..7]);
        assert_eq!(pig.cursor, 8);
    }

    #[test]
    fn test_skip_match() {
        let s = b"foo";
        let mut pig = Pig::new(s);
        assert!(pig.skip(b'f').is_ok());
        assert_eq!(pig.cursor, 1);
    }

    #[test]
    fn test_skip_no_match() {
        let s = b"foo";
        let mut pig = Pig::new(s);
        assert!(pig.skip(b'x').is_err());
        assert_eq!(pig.cursor, 0); // nothing consumed
    }

    #[test]
    fn test_skip_eos() {
        let s = b"f";
        let mut pig = Pig::new(s);
        assert!(pig.skip(b'f').is_ok());
        assert!(pig.skip(b'f').is_err());
    }
}
