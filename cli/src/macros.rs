#![macro_use]

/// create a &[&str] from vec notation
#[cfg(test)]
macro_rules! argv {
    () => (
        &[][..]
    );
    ($($x:expr),* $(,)?) => (
        &[$($x),*][..]
    );
}

/// Create a hashset, similar to vec!
#[cfg(test)]
macro_rules! set(
    { $($key:expr),+ } => {
        {
            let mut s = ::std::collections::HashSet::new();
            $(
                s.insert($key);
            )+
            s
        }
     };
);

/// Create a String from an &str; just a testing shorthand
#[cfg(test)]
macro_rules! s(
    { $s:expr } => { $s.to_owned() };
);

/// Create a Tag from an &str; just a testing shorthand
#[cfg(test)]
macro_rules! tag(
    { $s:expr } => { { use std::convert::TryFrom; taskchampion::Tag::try_from($s).unwrap() } };
);
