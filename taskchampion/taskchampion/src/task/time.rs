use chrono::{offset::LocalResult, DateTime, TimeZone, Utc};

pub type Timestamp = DateTime<Utc>;

pub fn utc_timestamp(secs: i64) -> Timestamp {
    match Utc.timestamp_opt(secs, 0) {
        LocalResult::Single(tz) => tz,
        // The other two variants are None and Ambiguous, which both are caused by DST.
        _ => unreachable!("We're requesting UTC so daylight saving time isn't a factor."),
    }
}
