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
