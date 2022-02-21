//! Parsers for single arguments (strings)

mod arg_matching;
mod colon;
mod idlist;
mod misc;
mod tags;
mod time;

pub(crate) use arg_matching::arg_matching;
pub(crate) use colon::{depends_colon, status_colon, wait_colon};
pub(crate) use idlist::{id_list, TaskId};
pub(crate) use misc::{any, literal, report_name};
pub(crate) use tags::{minus_tag, plus_tag};
#[allow(unused_imports)]
pub(crate) use time::{duration, timestamp};
