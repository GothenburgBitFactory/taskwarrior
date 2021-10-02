use chrono::{prelude::*, Duration};
use iso8601_duration::Duration as IsoDuration;
use lazy_static::lazy_static;
use nom::{
    branch::*,
    bytes::complete::*,
    character::complete::*,
    character::*,
    combinator::*,
    error::{Error, ErrorKind},
    multi::*,
    sequence::*,
    Err, IResult,
};
use std::str::FromStr;

// https://taskwarrior.org/docs/dates.html
// https://taskwarrior.org/docs/named_dates.html
// https://taskwarrior.org/docs/durations.html

/// A case for matching durations.  If `.3` is true, then the value can be used
/// without a prefix, e.g., `minute`.  If false, it cannot, e.g., `minutes`
#[derive(Debug)]
struct DurationCase(&'static str, Duration, bool);

// https://github.com/GothenburgBitFactory/libshared/blob/9a5f24e2acb38d05afb8f8e316a966dee196a42a/src/Duration.cpp#L50
// TODO: use const when chrono supports it
lazy_static! {
    static ref DURATION_CASES: Vec<DurationCase> = vec![
        DurationCase("days", Duration::days(1), false),
        DurationCase("day", Duration::days(1), true),
        DurationCase("d", Duration::days(1), false),
        DurationCase("hours", Duration::hours(1), false),
        DurationCase("hour", Duration::hours(1), true),
        DurationCase("h", Duration::hours(1), false),
        DurationCase("minutes", Duration::minutes(1), false),
        DurationCase("minute", Duration::minutes(1), true),
        DurationCase("mins", Duration::minutes(1), false),
        DurationCase("min", Duration::minutes(1), true),
        DurationCase("months", Duration::days(30), false),
        DurationCase("month", Duration::days(30), true),
        DurationCase("mo", Duration::days(30), true),
        DurationCase("seconds", Duration::seconds(1), false),
        DurationCase("second", Duration::seconds(1), true),
        DurationCase("s", Duration::seconds(1), false),
        DurationCase("weeks", Duration::days(7), false),
        DurationCase("week", Duration::days(7), true),
        DurationCase("w", Duration::days(7), false),
        DurationCase("years", Duration::days(365), false),
        DurationCase("year", Duration::days(365), true),
        DurationCase("y", Duration::days(365), false),
    ];
}

/// Parses suffixes like 'min', and 'd'; standalone is true if there is no numeric prefix, in which
/// case plurals (like `days`) are not matched.
fn duration_suffix(has_prefix: bool) -> impl Fn(&str) -> IResult<&str, Duration> {
    move |input: &str| {
        // Rust wants this to have a default value, but it is not actually used
        // because DURATION_CASES has at least one case with case.2 == `true`
        let mut res = Err(Err::Failure(Error::new(input, ErrorKind::Tag)));
        for case in DURATION_CASES.iter() {
            if !case.2 && !has_prefix {
                // this case requires a prefix, and input does not have one
                continue;
            }
            res = tag(case.0)(input);
            match res {
                Ok((i, _)) => {
                    return Ok((i, case.1));
                }
                Err(Err::Error(_)) => {
                    // recoverable error
                    continue;
                }
                Err(e) => {
                    // irrecoverable error
                    return Err(e);
                }
            }
        }

        // return the last error
        Err(res.unwrap_err())
    }
}
/// Calculate the multiplier for a decimal prefix; this uses integer math
/// where possible, falling back to floating-point math on seconds
fn decimal_prefix_multiplier(input: &str) -> IResult<&str, f64> {
    map_res(
        // recognize NN or NN.NN
        alt((recognize(tuple((digit1, char('.'), digit1))), digit1)),
        |input: &str| -> Result<f64, <f64 as FromStr>::Err> {
            let mul = input.parse::<f64>()?;
            Ok(mul)
        },
    )(input)
}

/// Parse an iso8601 duration, converting it to a [`chrono::Duration`] on the assumption
/// that a year is 365 days and a month is 30 days.
fn iso8601_dur(input: &str) -> IResult<&str, Duration> {
    if let Ok(iso_dur) = IsoDuration::parse(input) {
        // iso8601_duration uses f32, but f32 underflows seconds for values as small as
        // a year.  So we upgrade to f64 immediately.  f64 has a 53-bit mantissa which can
        // represent almost 300 million years without underflow, so it should be adequate.
        let days = iso_dur.year as f64 * 365.0 + iso_dur.month as f64 * 30.0 + iso_dur.day as f64;
        let hours = days * 24.0 + iso_dur.hour as f64;
        let mins = hours * 60.0 + iso_dur.minute as f64;
        let secs = mins * 60.0 + iso_dur.second as f64;
        let dur = Duration::seconds(secs as i64);
        Ok((&input[input.len()..], dur))
    } else {
        Err(Err::Error(Error::new(input, ErrorKind::Tag)))
    }
}

/// Recognizes durations
pub(crate) fn duration(input: &str) -> IResult<&str, Duration> {
    alt((
        map_res(
            tuple((
                decimal_prefix_multiplier,
                multispace0,
                duration_suffix(true),
            )),
            |input: (f64, &str, Duration)| -> Result<Duration, ()> {
                // `as i64` is saturating, so for large offsets this will
                // just pick an imprecise very-futuristic date
                let secs = (input.0 * input.2.num_seconds() as f64) as i64;
                Ok(Duration::seconds(secs))
            },
        ),
        duration_suffix(false),
        iso8601_dur,
    ))(input)
}

/// Parse a rfc3339 datestamp
fn rfc3339_timestamp(input: &str) -> IResult<&str, DateTime<Utc>> {
    if let Ok(dt) = DateTime::parse_from_rfc3339(input) {
        // convert to UTC and truncate seconds
        let dt = dt.with_timezone(&Utc).trunc_subsecs(0);
        Ok((&input[input.len()..], dt))
    } else {
        Err(Err::Error(Error::new(input, ErrorKind::Tag)))
    }
}

fn named_date<Tz: TimeZone>(
    now: DateTime<Utc>,
    local: Tz,
) -> impl Fn(&str) -> IResult<&str, DateTime<Utc>> {
    move |input: &str| {
        let local_today = now.with_timezone(&local).date();
        let remaining = &input[input.len()..];
        let day_index = local_today.weekday().num_days_from_monday();
        match input {
            "yesterday" => Ok((remaining, local_today - Duration::days(1))),
            "today" => Ok((remaining, local_today)),
            "tomorrow" => Ok((remaining, local_today + Duration::days(1))),
            // TODO: lots more!
            "eod" => Ok((remaining, local_today + Duration::days(1))),
            "sod" => Ok((remaining, local_today)),
            "eow" => Ok((
                remaining,
                local_today + Duration::days((6 - day_index).into()),
            )),
            "eoww" => Ok((
                remaining,
                local_today + Duration::days((5 - day_index).into()),
            )),
            "sow" => Ok((
                remaining,
                local_today + Duration::days((6 - day_index).into()),
            )),
            "soww" => Ok((
                remaining,
                local_today + Duration::days((7 - day_index).into()),
            )),
            _ => Err(Err::Error(Error::new(input, ErrorKind::Tag))),
        }
        .map(|(rem, dt)| (rem, dt.and_hms(0, 0, 0).with_timezone(&Utc)))
    }
}

/// recognize a digit
fn digit(input: &str) -> IResult<&str, char> {
    satisfy(|c| is_digit(c as u8))(input)
}

/// Parse yyyy-mm-dd as the given date, at the local midnight
fn yyyy_mm_dd<Tz: TimeZone>(local: Tz) -> impl Fn(&str) -> IResult<&str, DateTime<Utc>> {
    move |input: &str| {
        fn parse_int<T: FromStr>(input: &str) -> Result<T, <T as FromStr>::Err> {
            input.parse::<T>()
        }
        map_res(
            tuple((
                map_res(recognize(count(digit, 4)), parse_int::<i32>),
                char('-'),
                map_res(recognize(many_m_n(1, 2, digit)), parse_int::<u32>),
                char('-'),
                map_res(recognize(many_m_n(1, 2, digit)), parse_int::<u32>),
            )),
            |input: (i32, char, u32, char, u32)| -> Result<DateTime<Utc>, ()> {
                // try to convert, handling out-of-bounds months or days as an error
                let ymd = match local.ymd_opt(input.0, input.2, input.4) {
                    chrono::LocalResult::Single(ymd) => Ok(ymd),
                    _ => Err(()),
                }?;
                Ok(ymd.and_hms(0, 0, 0).with_timezone(&Utc))
            },
        )(input)
    }
}

/// Recognizes timestamps
pub(crate) fn timestamp<Tz: TimeZone + Copy>(
    now: DateTime<Utc>,
    local: Tz,
) -> impl Fn(&str) -> IResult<&str, DateTime<Utc>> {
    move |input: &str| {
        alt((
            // relative time
            map_res(
                duration,
                |duration: Duration| -> Result<DateTime<Utc>, ()> { Ok(now + duration) },
            ),
            rfc3339_timestamp,
            yyyy_mm_dd(local),
            value(now, tag("now")),
            named_date(now, local),
        ))(input)
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::argparse::NOW;
    use pretty_assertions::assert_eq;
    use rstest::rstest;

    const M: i64 = 60;
    const H: i64 = M * 60;
    const DAY: i64 = H * 24;
    const MONTH: i64 = DAY * 30;
    const YEAR: i64 = DAY * 365;

    // TODO: use const when chrono supports it
    lazy_static! {
        // India standard time (not an even multiple of hours)
        static ref IST: FixedOffset = FixedOffset::east(5 * 3600 + 30 * 60);
        // Utc, but as a FixedOffset TimeZone impl
        static ref UTC_FO: FixedOffset = FixedOffset::east(0);
        // Hawaii
        static ref HST: FixedOffset = FixedOffset::west(10 * 3600);
    }

    /// test helper to ensure that the entire input is consumed
    fn complete_duration(input: &str) -> IResult<&str, Duration> {
        all_consuming(duration)(input)
    }

    /// test helper to ensure that the entire input is consumed
    fn complete_timestamp<Tz: TimeZone + Copy>(
        now: DateTime<Utc>,
        local: Tz,
    ) -> impl Fn(&str) -> IResult<&str, DateTime<Utc>> {
        move |input: &str| all_consuming(timestamp(now, local))(input)
    }

    /// Shorthand day and time
    fn dt(y: i32, m: u32, d: u32, hh: u32, mm: u32, ss: u32) -> DateTime<Utc> {
        Utc.ymd(y, m, d).and_hms(hh, mm, ss)
    }

    /// Local day and time, parameterized on the timezone
    fn ldt(
        y: i32,
        m: u32,
        d: u32,
        hh: u32,
        mm: u32,
        ss: u32,
    ) -> Box<dyn Fn(FixedOffset) -> DateTime<Utc>> {
        Box::new(move |tz| tz.ymd(y, m, d).and_hms(hh, mm, ss).with_timezone(&Utc))
    }

    fn ld(y: i32, m: u32, d: u32) -> Box<dyn Fn(FixedOffset) -> DateTime<Utc>> {
        ldt(y, m, d, 0, 0, 0)
    }

    #[rstest]
    #[case::rel_hours_0(dt(2021, 5, 29, 1, 30, 0), "0h", dt(2021, 5, 29, 1, 30, 0))]
    #[case::rel_hours_05(dt(2021, 5, 29, 1, 30, 0), "0.5h", dt(2021, 5, 29, 2, 0, 0))]
    #[case::rel_hours_no_prefix(dt(2021, 5, 29, 1, 30, 0), "hour", dt(2021, 5, 29, 2, 30, 0))]
    #[case::rel_hours_5(dt(2021, 5, 29, 1, 30, 0), "5h", dt(2021, 5, 29, 6, 30, 0))]
    #[case::rel_days_0(dt(2021, 5, 29, 1, 30, 0), "0d", dt(2021, 5, 29, 1, 30, 0))]
    #[case::rel_days_10(dt(2021, 5, 29, 1, 30, 0), "10d", dt(2021, 6, 8, 1, 30, 0))]
    #[case::rfc3339_datetime(*NOW, "2019-10-12T07:20:50.12Z", dt(2019, 10, 12, 7, 20, 50))]
    #[case::now(*NOW, "now", *NOW)]
    /// Cases where the `local` parameter is ignored
    fn test_nonlocal_timestamp(
        #[case] now: DateTime<Utc>,
        #[case] input: &'static str,
        #[case] output: DateTime<Utc>,
    ) {
        let (_, res) = complete_timestamp(now, *IST)(input).unwrap();
        assert_eq!(res, output, "parsing {:?}", input);
    }

    #[rstest]
    /// Cases where the `local` parameter matters
    #[case::yyyy_mm_dd(ld(2000, 1, 1), "2021-01-01", ld(2021, 1, 1))]
    #[case::yyyy_m_d(ld(2000, 1, 1), "2021-1-1", ld(2021, 1, 1))]
    #[case::yesterday(ld(2021, 3, 1), "yesterday", ld(2021, 2, 28))]
    #[case::yesterday_from_evening(ldt(2021, 3, 1, 21, 30, 30), "yesterday", ld(2021, 2, 28))]
    #[case::today(ld(2021, 3, 1), "today", ld(2021, 3, 1))]
    #[case::today_from_evening(ldt(2021, 3, 1, 21, 30, 30), "today", ld(2021, 3, 1))]
    #[case::tomorrow(ld(2021, 3, 1), "tomorrow", ld(2021, 3, 2))]
    #[case::tomorow_from_evening(ldt(2021, 3, 1, 21, 30, 30), "tomorrow", ld(2021, 3, 2))]
    #[case::end_of_week(ld(2021, 8, 25,), "eow", ld(2021, 8, 29))]
    #[case::end_of_work_week(ld(2021, 8, 25), "eoww", ld(2021, 8, 28))]
    #[case::start_of_week(ld(2021, 8, 25), "sow", ld(2021, 8, 29))]
    #[case::start_of_work_week(ld(2021, 8, 25), "soww", ld(2021, 8, 30))]
    #[case::end_of_today(ld(2021, 8, 25), "eod", ld(2021, 8, 26))]
    #[case::start_of_today(ld(2021, 8, 25), "sod", ld(2021, 8, 25))]
    fn test_local_timestamp(
        #[case] now: Box<dyn Fn(FixedOffset) -> DateTime<Utc>>,
        #[values(*IST, *UTC_FO, *HST)] tz: FixedOffset,
        #[case] input: &str,
        #[case] output: Box<dyn Fn(FixedOffset) -> DateTime<Utc>>,
    ) {
        let now = now(tz);
        let output = output(tz);
        let (_, res) = complete_timestamp(now, tz)(input).unwrap();
        assert_eq!(
            res, output,
            "parsing {:?} relative to {:?} in timezone {:?}",
            input, now, tz
        );
    }

    #[rstest]
    #[case::rfc3339_datetime_bad_month(*NOW, "2019-10-99T07:20:50.12Z")]
    #[case::yyyy_mm_dd_bad_month(*NOW, "2019-10-99")]
    fn test_timestamp_err(#[case] now: DateTime<Utc>, #[case] input: &'static str) {
        let res = complete_timestamp(now, Utc)(input);
        assert!(
            res.is_err(),
            "expected error parsing {:?}, got {:?}",
            input,
            res.unwrap()
        );
    }

    // All test cases from
    // https://github.com/GothenburgBitFactory/libshared/blob/9a5f24e2acb38d05afb8f8e316a966dee196a42a/test/duration.t.cpp#L136
    #[rstest]
    #[case("0seconds", 0)]
    #[case("2 seconds", 2)]
    #[case("10seconds", 10)]
    #[case("1.5seconds", 1)]
    #[case("0second", 0)]
    #[case("2 second", 2)]
    #[case("10second", 10)]
    #[case("1.5second", 1)]
    #[case("0s", 0)]
    #[case("2 s", 2)]
    #[case("10s", 10)]
    #[case("1.5s", 1)]
    #[case("0minutes", 0)]
    #[case("2 minutes", 2 * M)]
    #[case("10minutes", 10 * M)]
    #[case("1.5minutes", M + 30)]
    #[case("0minute", 0)]
    #[case("2 minute", 2 * M)]
    #[case("10minute", 10 * M)]
    #[case("1.5minute", M + 30)]
    #[case("0min", 0)]
    #[case("2 min", 2 * M)]
    #[case("10min", 10 * M)]
    #[case("1.5min", M + 30)]
    #[case("0hours", 0)]
    #[case("2 hours", 2 * H)]
    #[case("10hours", 10 * H)]
    #[case("1.5hours", H + 30 * M)]
    #[case("0hour", 0)]
    #[case("2 hour", 2 * H)]
    #[case("10hour", 10 * H)]
    #[case("1.5hour", H + 30 * M)]
    #[case("0h", 0)]
    #[case("2 h", 2 * H)]
    #[case("10h", 10 * H)]
    #[case("1.5h", H + 30 * M)]
    #[case("0days", 0)]
    #[case("2 days", 2 * DAY)]
    #[case("10days", 10 * DAY)]
    #[case("1.5days", DAY + 12 * H)]
    #[case("0day", 0)]
    #[case("2 day", 2 * DAY)]
    #[case("10day", 10 * DAY)]
    #[case("1.5day", DAY + 12 * H)]
    #[case("0d", 0)]
    #[case("2 d", 2 * DAY)]
    #[case("10d", 10 * DAY)]
    #[case("1.5d", DAY + 12 * H)]
    #[case("0weeks", 0)]
    #[case("2 weeks", 14 * DAY)]
    #[case("10weeks", 70 * DAY)]
    #[case("1.5weeks", 10 * DAY + 12 * H)]
    #[case("0week", 0)]
    #[case("2 week", 14 * DAY)]
    #[case("10week", 70 * DAY)]
    #[case("1.5week", 10 * DAY + 12 * H)]
    #[case("0w", 0)]
    #[case("2 w", 14 * DAY)]
    #[case("10w", 70 * DAY)]
    #[case("1.5w", 10 * DAY + 12 * H)]
    #[case("0months", 0)]
    #[case("2 months", 60 * DAY)]
    #[case("10months", 300 * DAY)]
    #[case("1.5months", 45 * DAY)]
    #[case("0month", 0)]
    #[case("2 month", 60 * DAY)]
    #[case("10month", 300 * DAY)]
    #[case("1.5month", 45 * DAY)]
    #[case("0mo", 0)]
    #[case("2 mo", 60 * DAY)]
    #[case("10mo", 300 * DAY)]
    #[case("1.5mo", 45 * DAY)]
    #[case("0years", 0)]
    #[case("2 years", 2 * YEAR)]
    #[case("10years", 10 * YEAR)]
    #[case("1.5years", 547 * DAY + 12 * H)]
    #[case("0year", 0)]
    #[case("2 year", 2 * YEAR)]
    #[case("10year", 10 * YEAR)]
    #[case("1.5year", 547 * DAY + 12 * H)]
    #[case("0y", 0)]
    #[case("2 y", 2 * YEAR)]
    #[case("10y", 10 * YEAR)]
    #[case("1.5y", 547 * DAY + 12 * H)]
    fn test_duration_units(#[case] input: &'static str, #[case] seconds: i64) {
        let (_, res) = complete_duration(input).expect(input);
        assert_eq!(res.num_seconds(), seconds, "parsing {}", input);
    }

    #[rstest]
    #[case("years")]
    #[case("minutes")]
    #[case("eons")]
    #[case("P1S")] // missing T
    #[case("p1y")] // lower-case
    fn test_duration_errors(#[case] input: &'static str) {
        let res = complete_duration(input);
        assert!(
            res.is_err(),
            "did not get expected error parsing duration {:?}; got {:?}",
            input,
            res.unwrap()
        );
    }

    // https://github.com/GothenburgBitFactory/libshared/blob/9a5f24e2acb38d05afb8f8e316a966dee196a42a/test/duration.t.cpp#L115
    #[rstest]
    #[case("P1Y", YEAR)]
    #[case("P1M", MONTH)]
    #[case("P1D", DAY)]
    #[case("P1Y1M", YEAR + MONTH)]
    #[case("P1Y1D", YEAR + DAY)]
    #[case("P1M1D", MONTH + DAY)]
    #[case("P1Y1M1D", YEAR + MONTH + DAY)]
    #[case("PT1H", H)]
    #[case("PT1M", M)]
    #[case("PT1S", 1)]
    #[case("PT1H1M", H + M)]
    #[case("PT1H1S", H + 1)]
    #[case("PT1M1S", M + 1)]
    #[case("PT1H1M1S", H + M + 1)]
    #[case("P1Y1M1DT1H1M1S", YEAR + MONTH + DAY + H + M + 1)]
    #[case("PT24H", DAY)]
    #[case("PT40000000S", 40000000)]
    #[case("PT3600S", H)]
    #[case("PT60M", H)]
    fn test_duration_8601(#[case] input: &'static str, #[case] seconds: i64) {
        let (_, res) = complete_duration(input).expect(input);
        assert_eq!(res.num_seconds(), seconds, "parsing {}", input);
    }
}
