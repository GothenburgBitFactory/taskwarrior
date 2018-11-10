//! A minimal implementation of the "Nibbler" parsing utility from the Taskwarrior
//! source.

pub struct Nibbler<'a> {
    input: &'a [u8],
    cursor: usize,
}

impl<'a> Nibbler<'a> {
    pub fn new(input: &'a [u8]) -> Self {
        Nibbler {
            input: input,
            cursor: 0,
        }
    }

    pub fn get_until(&mut self, c: u8) -> Option<&'a [u8]> {
        if self.cursor >= self.input.len() {
            return None;
        }

        let mut i = self.cursor;
        while i < self.input.len() {
            if self.input[i] == c {
                let rv = &self.input[self.cursor..i];
                self.cursor = i;
                return Some(rv);
            }
            i += 1;
        }
        let rv = &self.input[self.cursor..];
        self.cursor = self.input.len();
        Some(rv)
    }

    // TODO: get_until_str
    // TODO: get_until_one_of
    // TODO: get_until_ws

    pub fn get_until_eos(&mut self) -> Option<&'a [u8]> {
        if self.cursor >= self.input.len() {
            return None;
        }
        let rv = &self.input[self.cursor..];
        self.cursor = self.input.len();
        return Some(rv);
    }

    // TODO: get_n

    pub fn get_quoted(&mut self, c: u8) -> Option<&'a [u8]> {
        let length = self.input.len();
        if self.cursor >= length || self.input[self.cursor] != c {
            return None;
        }

        let start = self.cursor + 1;
        let mut i = start;

        while i < length {
            while i < length && self.input[i] != c {
                i += 1
            }
            if i == length {
                // unclosed quote
                return None;
            }
            if i == start {
                return Some(&self.input[i..i]);
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
            return Some(&self.input[start..i]);
        }

        unreachable!();
    }

    // TODO: (missing funcs)

    pub fn skip_n(&mut self, n: usize) -> bool {
        let length = self.input.len();
        if self.cursor < length && self.cursor + n <= length {
            self.cursor += n;
            return true;
        }

        return false;
    }

    pub fn skip(&mut self, c: u8) -> bool {
        if self.cursor < self.input.len() && self.input[self.cursor] == c {
            self.cursor += 1;
            return true;
        }
        false
    }

    // TODO: skip_all_one_of
    // TODO: skip_ws

    pub fn next(&mut self) -> Option<u8> {
        if self.cursor >= self.input.len() {
            return None;
        }
        let rv = self.input[self.cursor];
        self.cursor += 1;
        return Some(rv);
    }

    // TODO: next_n
    // TODO: cursor
    // TODO: save
    // TODO: restore

    pub fn depleted(&self) -> bool {
        self.cursor >= self.input.len()
    }
}

#[cfg(test)]
mod test {
    use super::Nibbler;

    #[test]
    fn test_get_until() {
        let s = b"abc:123";
        let mut nib = Nibbler::new(s);
        assert_eq!(nib.get_until(b':'), Some(&s[..3]));
    }

    #[test]
    fn test_get_until_empty() {
        let s = b"abc:123";
        let mut nib = Nibbler::new(s);
        assert_eq!(nib.get_until(b'a'), Some(&s[..0]));
    }

    #[test]
    fn test_get_until_not_found() {
        let s = b"abc:123";
        let mut nib = Nibbler::new(s);
        assert_eq!(nib.get_until(b'/'), Some(&s[..]));
    }

    #[test]
    fn test_get_until_eos() {
        let s = b"abc:123";
        let mut nib = Nibbler::new(s);
        assert_eq!(nib.get_until_eos(), Some(&s[..]));
    }

    #[test]
    fn test_get_quoted() {
        let s = b"'abcd'efg";
        let mut nib = Nibbler::new(s);
        assert_eq!(nib.get_quoted(b'\''), Some(&s[1..5]));
        assert_eq!(nib.next(), Some(b'e'));
    }

    #[test]
    fn test_get_quoted_unopened() {
        let s = b"abcd'efg";
        let mut nib = Nibbler::new(s);
        assert_eq!(nib.get_quoted(b'\''), None);
        assert_eq!(nib.next(), Some(b'a')); // nothing consumed
    }

    #[test]
    fn test_get_quoted_unclosed() {
        let s = b"'abcdefg";
        let mut nib = Nibbler::new(s);
        assert_eq!(nib.get_quoted(b'\''), None);
        assert_eq!(nib.next(), Some(b'\'')); // nothing consumed
    }

    #[test]
    fn test_get_quoted_escaped() {
        let s = b"'abc\\'de'fg";
        let mut nib = Nibbler::new(s);
        assert_eq!(nib.get_quoted(b'\''), Some(&s[1..8]));
        assert_eq!(nib.next(), Some(b'f'));
    }

    #[test]
    fn test_get_quoted_double_escaped() {
        let s = b"'abc\\\\'de'fg";
        let mut nib = Nibbler::new(s);
        assert_eq!(nib.get_quoted(b'\''), Some(&s[1..6]));
        assert_eq!(nib.next(), Some(b'd'));
    }

    #[test]
    fn test_get_quoted_triple_escaped() {
        let s = b"'abc\\\\\\'de'fg";
        let mut nib = Nibbler::new(s);
        assert_eq!(nib.get_quoted(b'\''), Some(&s[1..10]));
        assert_eq!(nib.next(), Some(b'f'));
    }

    #[test]
    fn test_get_quoted_all_escapes() {
        let s = b"'\\\\\\'\\\\'fg";
        let mut nib = Nibbler::new(s);
        assert_eq!(nib.get_quoted(b'\''), Some(&s[1..7]));
        assert_eq!(nib.next(), Some(b'f'));
    }

    #[test]
    fn test_skip_n() {
        let s = b"abc:123";
        let mut nib = Nibbler::new(s);
        assert!(nib.skip_n(3));
        assert_eq!(nib.get_until_eos(), Some(&s[3..]));
    }

    #[test]
    fn test_skip_n_too_long() {
        let s = b"abc:123";
        let mut nib = Nibbler::new(s);
        assert!(!nib.skip_n(33));
        // nothing is consumed
        assert_eq!(nib.get_until_eos(), Some(&s[..]));
    }

    #[test]
    fn test_skip_n_exact_eos() {
        let s = b"abc:123";
        let mut nib = Nibbler::new(s);
        assert!(nib.skip_n(7));
        assert_eq!(nib.get_until_eos(), None);
    }

    #[test]
    fn test_skip_match() {
        let s = b"foo";
        let mut nib = Nibbler::new(s);
        assert!(nib.skip(b'f'));
        assert_eq!(nib.get_until_eos(), Some(&s[1..]));
    }

    #[test]
    fn test_skip_no_match() {
        let s = b"foo";
        let mut nib = Nibbler::new(s);
        assert!(!nib.skip(b'x'));
        assert_eq!(nib.get_until_eos(), Some(&s[..]));
    }

    #[test]
    fn test_skip_eos() {
        let s = b"foo";
        let mut nib = Nibbler::new(s);
        assert!(nib.skip_n(3));
        assert!(!nib.skip(b'x'));
    }

    #[test]
    fn test_next() {
        let s = b"foo";
        let mut nib = Nibbler::new(s);
        assert_eq!(nib.next(), Some(b'f'));
        assert_eq!(nib.next(), Some(b'o'));
        assert_eq!(nib.next(), Some(b'o'));
        assert_eq!(nib.next(), None);
        assert_eq!(nib.next(), None);
    }

    #[test]
    fn test_depleted() {
        let s = b"xy";
        let mut nib = Nibbler::new(s);
        assert!(!nib.depleted());
        assert_eq!(nib.next(), Some(b'x'));
        assert!(!nib.depleted());
        assert_eq!(nib.next(), Some(b'y'));
        assert!(nib.depleted());
    }
}
