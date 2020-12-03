//! Responsible for executing commands as parsed by [`crate::argparse`].

pub(crate) mod add;
pub(crate) mod gc;
pub(crate) mod help;
pub(crate) mod info;
pub(crate) mod list;
pub(crate) mod modify;
pub(crate) mod sync;
pub(crate) mod version;

#[cfg(test)]
mod test;
