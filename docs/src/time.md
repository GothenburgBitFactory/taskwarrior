## Timestamps

Times may be specified in a wide variety of convenient formats.

 * [RFC3339](https://datatracker.ietf.org/doc/html/rfc3339) timestamps, such as `2019-10-12 07:20:50.12Z`
 * A date of the format `YYYY-MM-DD` is interpreted as _local_ midnight on the given date.
   Single-digit month and day are accepted, but the year must contain four digits.
 * `now` refers to the exact current time
 * `yesterday`, `today`, and `tomorrow` refer to _local_ midnight on the given day
 * Any duration (described below) may be used as a timestamp, and is considered relative to the current time.

Times are stored internally as UTC.

## Durations

Durations can be given in a dizzying array of units.
Each can be preceded by a whole number or a decimal multiplier, e.g., `3days`.
The multiplier is optional with the singular forms of the units; for example `day` is allowed.
Some of the units allow an adjectival form, such as `daily` or `annually`; this form is more readable in some cases, but otherwise has the same meaning.

 * `s`, `sec`, `secs`, `second`, or `seconds`
 * `min`, `mins`, `minute`, or `minutes` (note that `m` is a month!)
 * `h`, `hr`, `hrs`, `hour`, or `hours`
 * `d`, `day`, `days`, `daily`, or `weekdays` (note, weekdays includes weekends!)
 * `w`, `wk`, `wks`, `week`, `weeks`, or `weekly`
 * `biweekly`, `fornight` or `sennight` (14 days)
 * `m`, `mo`, `mos`, `mth`, `mths`, `mnths`, `month`, `months`, or `monthly` (always 30 days, regardless of calendar month)
 * `binmonthly` (61 days)
 * `q`, `qtr`, `qtrs`, `qrtr`, `qrtrs`, `quarter`, `quarters`, or `quarterly` (91 days)
 * `semiannual` (183 days)
 * `y`, `yr`, `yrs`, `year`, `years`, `yearly`, or `annual` (365 days, regardless of leap days)
 * `biannual` or `biyearly` (730 days)

[ISO 8601 standard durations](https://en.wikipedia.org/wiki/ISO_8601#Durations) are also allowed.
While the standard does not specify the length of "P1Y" or "P1M", Taskchampion treats those as 365 and 30 days, respectively.

