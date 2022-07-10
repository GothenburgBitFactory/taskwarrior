#![macro_use]

/// Create a hashset, similar to vec!
// NOTE: in Rust 1.56.0, this can be changed to HashSet::from([..])
#[cfg(test)]
macro_rules! set(
    { $($key:expr),* $(,)? } => {
        {
            #[allow(unused_mut)]
            let mut s = ::std::collections::HashSet::new();
            $(
                s.insert($key);
            )*
            s
        }
     };
);
