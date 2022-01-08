//! Responsible for executing commands as parsed by [`crate::argparse`].

pub(crate) mod add;
pub(crate) mod config;
pub(crate) mod gc;
pub(crate) mod help;
pub(crate) mod import;
pub(crate) mod import_tdb2;
pub(crate) mod info;
pub(crate) mod modify;
pub(crate) mod report;
pub(crate) mod sync;
pub(crate) mod undo;
pub(crate) mod version;
