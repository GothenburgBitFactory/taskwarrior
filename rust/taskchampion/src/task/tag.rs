use std::convert::TryFrom;
use std::fmt;
use std::str::FromStr;

/// A Tag is a descriptor for a task, that is either present or absent, and can be used for
/// filtering.  Tags composed of all uppercase letters are reserved for synthetic tags.
///
/// Valid tags must not contain whitespace or any of the characters in `+-*/(<>^! %=~`.
/// The first characters additionally cannot be a digit, and subsequent characters cannot be `:`.
/// This definition is based on [that of
/// TaskWarrior](https://github.com/GothenburgBitFactory/taskwarrior/blob/663c6575ceca5bd0135ae884879339dac89d3142/src/Lexer.cpp#L146-L164).
#[derive(Clone, Eq, PartialEq, Ord, PartialOrd, Hash, Debug)]
pub struct Tag(TagInner);

/// Inner type to hide the implementation
#[derive(Clone, Eq, PartialEq, Ord, PartialOrd, Hash, Debug)]
pub(super) enum TagInner {
    User(String),
    Synthetic(SyntheticTag),
}

// see doc comment for Tag, above
pub const INVALID_TAG_CHARACTERS: &str = "+-*/(<>^! %=~";

impl Tag {
    /// True if this tag is a synthetic tag
    pub fn is_synthetic(&self) -> bool {
        matches!(self.0, TagInner::Synthetic(_))
    }

    /// True if this tag is a user-provided tag (not synthetic)
    pub fn is_user(&self) -> bool {
        matches!(self.0, TagInner::User(_))
    }

    pub(super) fn inner(&self) -> &TagInner {
        &self.0
    }

    pub(super) fn from_inner(inner: TagInner) -> Self {
        Self(inner)
    }
}

impl FromStr for Tag {
    type Err = anyhow::Error;

    fn from_str(value: &str) -> Result<Tag, anyhow::Error> {
        fn err(value: &str) -> Result<Tag, anyhow::Error> {
            anyhow::bail!("invalid tag {:?}", value)
        }

        // first, look for synthetic tags
        if value.chars().all(|c| c.is_ascii_uppercase()) {
            if let Ok(st) = SyntheticTag::from_str(value) {
                return Ok(Self(TagInner::Synthetic(st)));
            }
            // all uppercase, but not a valid synthetic tag
            return err(value);
        }

        if let Some(c) = value.chars().next() {
            if c.is_whitespace() || c.is_ascii_digit() || INVALID_TAG_CHARACTERS.contains(c) {
                return err(value);
            }
        } else {
            return err(value);
        }
        if !value
            .chars()
            .skip(1)
            .all(|c| !(c.is_whitespace() || c == ':' || INVALID_TAG_CHARACTERS.contains(c)))
        {
            return err(value);
        }
        Ok(Self(TagInner::User(String::from(value))))
    }
}

impl TryFrom<&str> for Tag {
    type Error = anyhow::Error;

    fn try_from(value: &str) -> Result<Tag, Self::Error> {
        Self::from_str(value)
    }
}

impl TryFrom<&String> for Tag {
    type Error = anyhow::Error;

    fn try_from(value: &String) -> Result<Tag, Self::Error> {
        Self::from_str(&value[..])
    }
}

impl fmt::Display for Tag {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match &self.0 {
            TagInner::User(s) => s.fmt(f),
            TagInner::Synthetic(st) => st.as_ref().fmt(f),
        }
    }
}

impl AsRef<str> for Tag {
    fn as_ref(&self) -> &str {
        match &self.0 {
            TagInner::User(s) => s.as_ref(),
            TagInner::Synthetic(st) => st.as_ref(),
        }
    }
}

/// A synthetic tag, represented as an `enum`.  This type is used directly by
/// [`taskchampion::task::task`] for efficiency.
#[derive(
    Debug,
    Clone,
    Eq,
    PartialEq,
    Ord,
    PartialOrd,
    Hash,
    strum_macros::EnumString,
    strum_macros::AsRefStr,
    strum_macros::EnumIter,
)]
#[strum(serialize_all = "SCREAMING_SNAKE_CASE")]
pub(super) enum SyntheticTag {
    // When adding items here, also implement and test them in `task.rs` and document them in
    // `docs/src/tags.md`.
    Waiting,
    Active,
    Pending,
    Completed,
    Deleted,
    Blocked,
    Unblocked,
    Blocking,
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;
    use rstest::rstest;
    use std::convert::TryInto;

    #[rstest]
    #[case::simple("abc")]
    #[case::colon_prefix(":abc")]
    #[case::letters_and_numbers("a123_456")]
    #[case::synthetic("WAITING")]
    fn test_tag_try_into_success(#[case] s: &'static str) {
        let tag: Tag = s.try_into().unwrap();
        // check Display (via to_string) and AsRef while we're here
        assert_eq!(tag.to_string(), s.to_owned());
        assert_eq!(tag.as_ref(), s);
    }

    #[rstest]
    #[case::empty("")]
    #[case::colon_infix("a:b")]
    #[case::digits("999")]
    #[case::bangs("abc!!!")]
    #[case::no_such_synthetic("NOSUCH")]
    fn test_tag_try_into_err(#[case] s: &'static str) {
        let tag: Result<Tag, _> = s.try_into();
        assert_eq!(
            tag.unwrap_err().to_string(),
            format!("invalid tag \"{}\"", s)
        );
    }
}
