//! Re-implementation of TaskWarrior's A2 module.

use crate::util::lexer::*;
use std::collections::{HashMap, HashSet};
use std::fmt;

/// A2 represents a single argument.
#[derive(Clone)]
pub(crate) struct A2 {
    pub(crate) lextype: Type,
    tags: HashSet<String>,
    attributes: HashMap<String, String>,
}

impl A2 {
    pub(crate) fn new<S: Into<String>>(raw: S, lextype: Type) -> A2 {
        let mut attributes = HashMap::new();
        attributes.insert("raw".into(), raw.into());
        let mut rv = A2 {
            lextype,
            tags: HashSet::new(),
            attributes,
        };
        rv.decompose();
        rv
    }

    /// Return true if the given tag exists in this argument.
    pub(crate) fn has_tag<S: AsRef<str>>(&self, tag: S) -> bool {
        self.tags.contains(tag.as_ref())
    }

    /// Add the given tag to this argument.
    pub(crate) fn tag<S: Into<String>>(&mut self, tag: S) {
        self.tags.insert(tag.into());
    }

    /// Remove the given tag from this argument.
    pub(crate) fn untag<S: AsRef<str>>(&mut self, tag: S) {
        self.tags.remove(tag.as_ref());
    }

    /// Set the given attribute
    pub(crate) fn set_attribute<S1: Into<String>, S2: Into<String>>(
        &mut self,
        name: S1,
        value: S2,
    ) {
        self.attributes.insert(name.into(), value.into());
    }

    /// Get the given attribute
    pub(crate) fn get_attribute<S: AsRef<str>>(&self, name: S) -> Option<&str> {
        self.attributes.get(name.as_ref()).map(|s| s.as_ref())
    }

    /// Get either the canonical or raw form (attribute)
    pub(crate) fn get_token(&self) -> &str {
        self.attributes
            .get("canonical")
            .or_else(|| self.attributes.get("raw"))
            .unwrap()
            .as_ref()
    }

    /// Decompose the raw form into tags and attributes based on the lextype:
    ///
    /// * Tag -
    ///   - "name" is the tag name
    ///   - "sign" is the sign (`+` or `-`)
    /// * Substitution
    ///   - "from" is the first part
    ///   - "to" is the second part
    ///   - "flags' is the substitution flag, or empty string
    /// * Pair
    ///   - "name"
    ///   - "modifier"
    ///   - "separator"
    ///   - "value" are the parts of the pair (a pair has four parts..?)
    ///   - tag "RC" is set if the name is "rc" with no modifier
    ///   - tag "CONFIG" is set if the name is "rc" with a monitor
    /// * Pattern
    ///   - "pattern" is the pattern value
    ///   - "flags" is the pattern flag, or empty string
    ///
    /// all other types are left unchanged
    pub(crate) fn decompose(&mut self) {
        let raw = self.get_attribute("raw").unwrap();
        match self.lextype {
            Type::Tag => {
                let (sign, name) = (raw[..1].to_string(), raw[1..].to_string());
                self.set_attribute("sign", sign);
                self.set_attribute("name", name);
            }
            Type::Substitution => {
                let DecomposedSubstitution { from, to, flags } =
                    decompose_substitution(raw).unwrap();
                self.set_attribute("from", from);
                self.set_attribute("to", to);
                self.set_attribute("flags", flags);
            }
            Type::Pair => {
                let DecomposedPair {
                    name,
                    modifier,
                    separator,
                    value,
                } = decompose_pair(raw).unwrap();

                if &name == "rc" {
                    if &modifier != "" {
                        self.tag("CONFIG");
                    } else {
                        self.tag("RC");
                    }
                }

                self.set_attribute("name", name);
                self.set_attribute("modifier", modifier);
                self.set_attribute("separator", separator);
                self.set_attribute("value", value);
            }
            Type::Pattern => {
                let DecomposedPattern { pattern, flags } = decompose_pattern(raw).unwrap();

                self.set_attribute("pattern", pattern);
                self.set_attribute("flags", flags);
            }
            _ => (),
        }
    }
}

impl fmt::Debug for A2 {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "A2{}{:?}", "{", self.lextype)?;
        let mut tags = self.tags.iter().collect::<Vec<_>>();
        tags.sort();
        for tag in tags {
            write!(f, ", {}", tag)?;
        }
        let mut attributes = self.attributes.iter().collect::<Vec<_>>();
        attributes.sort();
        for (name, value) in attributes {
            write!(f, ", {}={:?}", name, value)?;
        }
        write!(f, "{}", "}")?;
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn tags() {
        let mut a2 = A2::new("ident", Type::Identifier);
        assert!(!a2.has_tag("foo"));
        a2.tag("foo");
        assert!(a2.has_tag("foo"));
        a2.untag("foo");
        assert!(!a2.has_tag("foo"));
    }

    #[test]
    fn raw_attribute() {
        let a2 = A2::new("ident", Type::Identifier);
        assert_eq!(a2.get_attribute("raw"), Some("ident"));
    }

    #[test]
    fn set_get_attribute() {
        let mut a2 = A2::new("ident", Type::Identifier);
        assert_eq!(a2.get_attribute("foo"), None);
        a2.set_attribute("foo", "bar");
        assert_eq!(a2.get_attribute("foo"), Some("bar"));
        a2.set_attribute("foo", "bing");
        assert_eq!(a2.get_attribute("foo"), Some("bing"));
    }

    #[test]
    fn get_token_raw() {
        let a2 = A2::new("ident", Type::Identifier);
        assert_eq!(a2.get_token(), "ident");
    }

    #[test]
    fn get_token_canonical() {
        let mut a2 = A2::new("ident", Type::Identifier);
        a2.set_attribute("canonical", "identifier");
        assert_eq!(a2.get_token(), "identifier");
    }

    #[test]
    fn decompose_tag() {
        let mut a2 = A2::new("+foo", Type::Tag);
        a2.decompose();
        assert_eq!(a2.get_attribute("sign"), Some("+"));
        assert_eq!(a2.get_attribute("name"), Some("foo"));
    }

    #[test]
    fn decompose_substitution() {
        let mut a2 = A2::new("/foo/bar/g", Type::Substitution);
        a2.decompose();
        assert_eq!(a2.get_attribute("from"), Some("foo"));
        assert_eq!(a2.get_attribute("to"), Some("bar"));
        assert_eq!(a2.get_attribute("flags"), Some("g"));
    }

    #[test]
    fn decompose_pair() {
        let mut a2 = A2::new("thing.foo:bar", Type::Pair);
        a2.decompose();
        assert_eq!(a2.get_attribute("name"), Some("thing"));
        assert_eq!(a2.get_attribute("modifier"), Some("foo"));
        assert_eq!(a2.get_attribute("separator"), Some(":"));
        assert_eq!(a2.get_attribute("value"), Some("bar"));
        assert!(!a2.has_tag("RC"));
        assert!(!a2.has_tag("CONFIG"));
    }

    #[test]
    fn decompose_pair_rc() {
        let mut a2 = A2::new("rc:bar", Type::Pair);
        a2.decompose();
        assert_eq!(a2.get_attribute("name"), Some("rc"));
        assert_eq!(a2.get_attribute("modifier"), Some(""));
        assert_eq!(a2.get_attribute("separator"), Some(":"));
        assert_eq!(a2.get_attribute("value"), Some("bar"));
        assert!(a2.has_tag("RC"));
        assert!(!a2.has_tag("CONFIG"));
    }

    #[test]
    fn decompose_pair_config() {
        let mut a2 = A2::new("rc.foo:bar", Type::Pair);
        a2.decompose();
        assert_eq!(a2.get_attribute("name"), Some("rc"));
        assert_eq!(a2.get_attribute("modifier"), Some("foo"));
        assert_eq!(a2.get_attribute("separator"), Some(":"));
        assert_eq!(a2.get_attribute("value"), Some("bar"));
        assert!(!a2.has_tag("RC"));
        assert!(a2.has_tag("CONFIG"));
    }

    #[test]
    fn decompose_pattern() {
        let mut a2 = A2::new("/foobar/g", Type::Pattern);
        a2.decompose();
        assert_eq!(a2.get_attribute("pattern"), Some("foobar"));
        assert_eq!(a2.get_attribute("flags"), Some("g"));
    }

    #[test]
    fn decompose_other() {
        let mut a2 = A2::new("123", Type::Number);
        a2.decompose();
        assert_eq!(a2.get_attribute("raw"), Some("123"));
    }

    #[test]
    fn debug() {
        let mut a2 = A2::new("/ab/g", Type::Pattern);
        a2.decompose();
        a2.tag("FOO");
        assert_eq!(
            format!("{:?}", a2),
            "A2{Pattern, FOO, flags=\"g\", pattern=\"ab\", raw=\"/ab/g\"}"
        );
    }
}
