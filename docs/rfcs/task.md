---
title: "Taskwarrior - Taskwarrior JSON Format"
---


# Taskwarrior JSON Format

When Taskwarrior exchanges data, it uses [JSON](https://www.json.org/).
This document describes the structure and semantics for tasks exported from Taskwarrior, imported to Taskwarrior, or synced with the Taskserver.

Any client of the Taskserver will need to communicate task information.
This document describes the format of a single task.
It does not describe the communication and sync protocol between client and server.

This document is subject to change.
The data attributes are also subject to change.


## Requirements

In this document, we adopt the convention discussed in Section 1.3.2 of [RFC1122](https://tools.ietf.org/html/rfc1122#page-16) of using the capitalized words MUST, REQUIRED, SHOULD, RECOMMENDED, MAY, and OPTIONAL to define the significance of each particular requirement specified in this document.

In brief: "MUST" (or "REQUIRED") means that the item is an absolute requirement of the specification; "SHOULD" (or "RECOMMENDED") means there may exist valid reasons for ignoring this item, but the full implications should be understood before doing so; and "MAY" (or "OPTIONAL") means that this item is optional, and may be omitted without careful consideration.


## General Format

The format is JSON, specifically a JSON object as a single line of text, terminated by a newline (U+000D).

The JSON looks like this:

    {"description":"One two three","status":"pending", ... }

While this is not a valid task (there are missing fields), the format is illustrated.

All attribute names are quoted with " (U+0022).
A name will always have a corresponding value, and if a value is blank, then the name/value pair is omitted from the line.
Newline characters are not permitted within the value, meaning that a task consists of a single line of text.

All data is UTF8.


## Data Types

There are five data types used in the task format.


## Data Type: String

Strings may consist of any UTF8 encoded characters.


## Data Type: Fixed String

A fixed string is one value from a set of acceptable values, such as a priority level, where the values may only be "", "L", "M" or "H".


## Data Type: UUID

A UUID is a 32-hex-character lower case string, formatted in this way:

    xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx

An example:

    296d835e-8f85-4224-8f36-c612cad1b9f8


## Data Type: Integer

Integers are rendered in a simple fashion:

    123


## Data Type: Date

Dates are rendered in ISO 8601 combined date and time in UTC format using the template:

    YYYYMMDDTHHMMSSZ

An example:

    20120110T231200Z

No other formats are supported.


## Data Type: Duration

Duration values represent a time period.
They take the form:

    [[<sign>] <number>] <unit>

Some examples include:

-   -3days
-   annual
-   4hrs

The supported units are:

-   annual
-   biannual
-   bimonthly
-   biweekly
-   biyearly
-   daily
-   days
-   day
-   d
-   fortnight
-   hours
-   hour
-   hrs
-   hr
-   h
-   minutes
-   mins
-   min
-   monthly
-   months
-   month
-   mnths
-   mths
-   mth
-   mos
-   mo
-   quarterly
-   quarters
-   qrtrs
-   qtrs
-   qtr
-   q
-   seconds
-   secs
-   sec
-   s
-   semiannual
-   sennight
-   weekdays
-   weekly
-   weeks
-   week
-   wks
-   wk
-   w
-   yearly
-   years
-   year
-   yrs
-   yr
-   y

Note that some values lack precision, for example "2q" means two quarters, or half a year.

Note that not all combinations of number and unit make sense, for example "3annual" makes no sense, but evaluates to "3years".


## The Attributes

Here are the standard attributes that may comprise a task:

| Name         | Type    |
|--------------|---------|
| status       | String  |
| uuid         | UUID    |
| entry        | Date    |
| description  | String  |
| start        | Date    |
| end          | Date    |
| due          | Date    |
| until        | Date    |
| wait         | Date    |
| modified     | Date    |
| scheduled    | Date    |
| recur        | String  |
| mask         | String  |
| imask        | Integer |
| parent       | UUID    |
| project      | String  |
| priority     | String  |
| depends      | String  |
| tags *       | String  |
| annotation * | String  |
| (UDA)        | ?       |

\* Both tags and annotations are lists of strings and objects.

Any UDA fields are assumed to be of type string.

There are other forms, which are conditional upon the state of a task:

| Status Value | Pending | Deleted | Completed | Waiting | Recurring Parent | Recurring Child |
|--------------|---------|---------|-----------|---------|------------------|-----------------|
| status       | Reqd    | Reqd    | Reqd      | Reqd    | Reqd             | Reqd            |
| uuid         | Reqd    | Reqd    | Reqd      | Reqd    | Reqd             | Reqd            |
| entry        | Reqd    | Reqd    | Reqd      | Reqd    | Reqd             | Reqd            |
| description  | Reqd    | Reqd    | Reqd      | Reqd    | Reqd             | Reqd            |
| start        | Opt     | Opt     | Opt       | Opt     | Opt              | Opt             |
| end          |         | Reqd    | Reqd      |         |                  |                 |
| due          | Opt     | Opt     | Opt       | Opt     | Reqd             | Opt             |
| until        | Opt     | Opt     | Opt       | Opt     | Opt              | Opt             |
| scheduled    | Opt     | Opt     | Opt       | Opt     | Opt              | Opt             |
| wait         |         |         |           | Reqd    |                  |                 |
| recur        |         |         |           |         | Reqd             | Reqd            |
| mask         |         |         |           |         | Intrn            |                 |
| imask        |         |         |           |         |                  | Intrn           |
| parent       |         |         |           |         |                  | Reqd            |
| annotation   | Opt     | Opt     | Opt       | Opt     | Opt              | Opt             |
| project      | Opt     | Opt     | Opt       | Opt     | Opt              | Opt             |
| tags         | Opt     | Opt     | Opt       | Opt     | Opt              | Opt             |
| priority     | Opt     | Opt     | Opt       | Opt     | Opt              | Opt             |
| depends      | Opt     | Opt     | Opt       | Opt     | Opt              | Opt             |
| modified     | Intrn   | Intrn   | Intrn     | Intrn   | Intrn            | Intrn           |
| UDA          | Opt     | Opt     | Opt       | Opt     | Opt              | Opt             |

(Legend: Reqd = required, Opt = optional, Intrn = Internally generated)

All tasks have four required fields.
There are other states in which a task may exist, and the requirements change.
At a minimum, a valid task contains:

-   uuid
-   status
-   entry
-   description

*Deleted* - A deleted task MUST also have "status":"deleted", an "end" date and a "modified" date.

*Completed* - A completed task MUST also have "status":"completed", an "end" date and a "modified" date.

*Waiting* - A waiting task MUST also have "status":"waiting" and a "wait" date.
The task is hidden from the user, until that "wait" date has passed, whereupon the status reverts to "pending", and the "wait" date is removed.

*Recurring Parent* - When a recurring task is entered, it MUST have "status":"recurring", a "recur" period and a "due" date.
It MAY also have an "until" date.
Recurring parent tasks are hidden from the user.

*Recurring Child* - A recurring child task is not created by the user, but is cloned from the recurring parent task by the Taskserver.
It may be modified by the user.
On completion, there is special handling to be done.
See section 3.11.


## Additional Attributes

There MAY be other fields than those listed above in a task definition.
Such fields MUST be preserved intact by any client, which means that if a task is downloaded that contains an unrecognized field, that field MUST not be modified, and MUST continue to exist in the task..

User Defined Attributes (UDAs) are additional fields.


## Attribute Details

The individual fields convey important information about a task, and in some cases work only in collusion with other fields.
All such details are listed here.


## Attribute: status

The status field describes the state of the task, which may ONLY be one of these literal strings:

    "status":"pending"
    "status":"deleted"
    "status":"completed"
    "status":"waiting"
    "status":"recurring"

A pending task is a task that has not yet been completed or deleted.
This is the typical state for a task.

A deleted task is one that has been removed from the pending state, and MUST have an "end" field specified.
Given the required "entry" and "end" field, it can be determined how long the task was pending.

A completed task is one that has been removed from the pending state by completion, and MUST have an "end" field specified.
Given the required "entry" and "end" fields, it can be determine how long the task was pending.

A waiting task is ostensibly a pending task that has been hidden from typical view, and MUST have a "wait" field containing the date when the task is automatically returned to the pending state.
If a client sees a task that is in the waiting state, and the "wait" field is earlier than the current date and time, the client MUST remove the "wait" field and set the "status" field to "pending".

A recurring task is essentially a parent template task from which child tasks are cloned.
The parent remains hidden from view, and contains a "mask" field that represents the recurrences.
Each cloned child task has an "imask" field that indexes into the parent "mask" field, as well as a "parent" field that lists the UUID of the parent.


## Attribute: uuid

When a task is created, it MUST be assigned a new UUID by the client.
Once assigned, a UUID field MUST NOT be modified.
UUID fields are permanent.


## Attribute: entry

When a task is created, it MUST be assigned an "entry" date by the client.
This is the creation date of the task.


## Attribute: description

When a task is created, it MUST have a "description" field value, which contains UTF8 characters.
A "description" field may not contain newline characters, but may contain other characters, properly escaped.
See <https://json.org> for details.


## Attribute: start

To indicate that a task is being worked on, it MAY be assigned a "start" field.
Such a task is then considered Active.


## Attribute: end

When a task is deleted or completed, is MUST be assigned an "end" field.
It is not valid for a task to have an "end" field unless the status is also "completed" or "deleted".
If a completed task is restored to the "pending" state, the "end" field is removed.


## Attribute: due

A task MAY have a "due" field, which indicates when the task should be completed.


## Attribute: until

A recurring task MAY have an "until" field, which is the date after which no more recurring tasks should be generated.
At that time, the parent recurring task is set to "completed".


## Attribute: wait

A task MAY have a "wait" field date, in conjunction with a "status" of "waiting".
A waiting task is one that is not typically shown on reports until it is past the wait date.

An example of this is a birthday reminder.
A task may be entered for a birthday reminder in 10 months time, but can have a "wait" date 9 months from now, which means the task remains hidden until 1 month before the due date.
This prevents long-term tasks from cluttering reports until they become relevant.


## Attribute: recur

The "recur" field is for recurring tasks, and specifies the period between child tasks, in the form of a duration value.
The value is kept in the raw state (such as "3wks") as a string, so that it may be evaluated each time it is needed.


## Attribute: mask

A parent recurring task has a "mask" field that is an array of child status indicators.
Suppose a task is created that is due every week for a month.
The "mask" field will look like:

    "----"

This mask has four slots, indicating that there are four child tasks, and each slot indicates, in this case, that the child tasks are pending ("-").
The possible slot indicators are:

* `-` - Pending
* `+` - Completed
* `X` - Deleted
* `W` - Waiting

Suppose the first three tasks has been completed, the mask would look like this:

    "+++-"

If there were only three indicators in the mask:

    "+-+"

This would indicate that the second task is pending, the first and third are complete, and the fourth has not yet been generated.


## Attribute: imask

Child recurring tasks have an "imask" field instead of a "mask" field like their parent.
The "imask" field is a zero-based integer offset into the "mask" field of the parent.

If a child task is completed, one of the changes that MUST occur is to look up the parent task, and using "imask" set the "mask" of the parent to the correct indicator.
This prevents recurring tasks from being generated twice.


## Attribute: parent

A recurring task instance MUST have a "parent" field, which is the UUID of the task that has "status" of "recurring".
This linkage between tasks, established using "parent", "mask" and "imask" is used to track the need to generate more recurring tasks.


## Attribute: annotation\_\...

Annotations are strings with timestamps.
Each annotation itself has an "entry" field and a "description" field, similar to the task itself.
Annotations form an array named "annotations".
For example (lines broken for clarity):

    "annotations":[
      {"entry":"20120110T234212Z","description":"Remember to get the mail"},
      {"entry":"20120110T234559Z","description":"Pay the bills"}
    ]


## Attribute: project

A project is a single string.
For example:

    "project":"Personal Taxes"

Note that projects receive special handling, so that when a "." (U+002E) is used, it implies a hierarchy, which means the following two projects:

    "Home.Kitchen"
    "Home.Garden"

are both considered part of the "Home" project.


## Attribute: tags

The "tags" field is an array of string, where each string is a single word containing no spaces.
For example:

    "tags":["home","garden"]


## Attribute: priority

The "priority" field, if present, MAY contain one of the following strings:

    "priority":"H"
    "priority":"M"
    "priority":"L"

These represent High, Medium and Low priorities.
An absent priority field indicates no priority.


## Attribute: depends

The "depends" field is a string containing a comma-separated unique set of UUIDs.
If task 2 depends on task 1, then it is task 1 that must be completed first.
Task 1 is considered a "blocking" tasks, and task 2 is considered a "blocked" task.
For example:

    "depends":",, ..."

Note that in a future version of this specification, this will be changed to a JSON array of strings, like the "tags" field.


## Attribute: modified

A task MUST have a "modified" field set if it is modified.
This field is of type "date", and is used as a reference when merging tasks.


## Attribute: scheduled

A task MAY have a "scheduled" field, which indicates when the task should be available to start.
A task that has passed its "scheduled" data is said to be "ready".


## User Defined Attributes

A User Defined Attribute (UDA) is a field that is defined via configuration.
Given that the configuration is not present in the JSON format of a task, any fields that are not recognized are to be treated as UDAs.
This means that if a task contains a UDA, unless the meaning of it is understood, it MUST be preserved.

UDAs may have one of four types: string, numeric, date and duration.

