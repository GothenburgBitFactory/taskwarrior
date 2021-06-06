#![allow(clippy::module_inception)]
use chrono::prelude::*;

mod annotation;
mod priority;
mod status;
mod tag;
mod task;

pub use annotation::Annotation;
pub use priority::Priority;
pub use status::Status;
pub use tag::{Tag, INVALID_TAG_CHARACTERS};
pub use task::{Task, TaskMut};

use tag::SyntheticTag;

pub type Timestamp = DateTime<Utc>;
