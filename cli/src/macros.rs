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
