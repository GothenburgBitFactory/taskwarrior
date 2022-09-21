---
title: "Taskwarrior - Full DOM Support"
---

## Work in Progress

This design document is a work in progress, and subject to change.
Once finalized, the feature will be scheduled for an upcoming release.


# Full DOM Support

Taskwarrior currently supports DOM references that can access any stored data item.
The general forms supported are:

    [ <id> | <uuid> ] <attribute> [ <part> ]

Examples include:

    due
    123.uuid
    entry.month
    123.annotations.0.entry.year
    a87bc10f-931b-4558-a44a-e901a77db011.description

Additionally there are references for accessing configuration and system/program level items.

    rc.<name>
    context.program
    context.args
    context.width
    context.height
    system.version
    system.os

While this is adequate for data retrieval, we have the possibility of extending it further to include data formats, higher-level constructs, and then to make use of DOM references in more locations.
This contributes to our goal of simplifying Taskwarrior.


## Proposed Format Support

When defining a custom report, the columns shown are defined like this:

    report.x.columns=uuid.short,description.oneline ...

This syntax is:

    <attribute> [ . <format> ]

If no `format` is specified, then `default` is assumed.
The `src/columns/ColΧ\*` objects are responsible for supporting and rendering these formats.
There is currently no consistency among these formats based on data type.

By incorporating formats into DOM references, we eliminate the need for a separate syntax for custom reports, and provide this:

    123.due.iso
    123.due.month.short
    123.uuid.short

A standard set of formats per data type would be:

Type

Formats

Example

Numeric

default

`123                              `

indicator

Based on `rc.<attribute>.indicator` which overrides `rc.numeric.indicator`.

json

`"<attribute>":"<value>"`

String

default

Buy milk

short

Feb

indicator

Based on `rc.<attribute>.indicator` which overrides `rc.string.indicator`.

json

`"<attribute>":"<value>"`

Date

default

Based on `rc.dateformat`

iso

2017-02-20T09:02:12

julian

2457805.12858

epoch

1234567890

age

2min

relative

-2min

remaining

0:02:04

countdown

0:02:04

indicator

Based on `rc.<attribute>.indicator` which overrides `rc.date.indicator`.

json

`"<attribute>":"<value>"`

Duration

default

1wk

iso

P1W

indicator

Based on `rc.<attribute>.indicator` which overrides `rc.duration.indicator`.

json

`"<attribute>":"<value>"`

There will also be a set of attribute-specific formats, similar to the currently supported set:

    depends.list
    depends.count
    description.combined
    description.desc
    description.oneline
    description.truncated
    description.count
    description.truncated_count
    parent.default|long
    parent.short
    project.full
    project.parent
    project.indented
    status.default|long
    status.short
    tags.default|list
    tags.count
    urgency.default|real
    urgency.integer
    uuid.default|long
    uuid.short

Custom report sort criteria will also use DOM references.
This will be augmented by the `+`/`-` sort direction and `/` break indicator, which are not part of the DOM.


## High Level Construct Support

There need to be read-only DOM references that do not correspond directly to stored attributes.
Tasks have emergent properties represented by virtual tags, which will be accessible, in this case returning a `0` or `1`:

    123.tags.OVERDUE

Using `rc.due` and the `due` attribute, the `OVERDUE` virtual tag is a combination of the two.
Other examples may include:

    task.syncneeded
    task.pending.count
    task.hooks.installed


## Writable References

When a DOM reference refers to an attribute or RC setting, and does not extend further and reference a component or format, it may be writable.
For example:

    rc.hooks           # writable
    123.description    # writable
    123.entry.month    # not writable, not an attribute


## Data Interchange

The export command can be used to show a filtered set of tasks in JSON format, and this will also be available as a DOM format:

    123.json
    a87bc10f-931b-4558-a44a-e901a77db011.json


## RC File Support

The RC file (`~/.taskrc`) will support DOM references in values.
This will form a late-bound reference, which is evaluated at runtime, every time.

An example is to make two reports share the same description:

    $ task config -- report.ls.description rc.report.list.description

This sets the description for the `ls` report to be a reference to the description of the `list` report. 
This reference is not evaluated when the entry is written, but is evaluated every time the value is read, thus providing late-bound behavior.
Then if the description of the `list` report changes, so does that of the `ls` report automatically.


## Implementation Details

These notes list a series of anticipated changes to the codebase.  

-   The `src/columns/Col*` objects will implement type-specific and attribute-specific DOM support.
    DOM reference lookup will defer to the column objects first.

-   Some DOM references will be writable, permitting a `_set` command to complement the `_get` command.

-   The `Config` object will recognize DOM references in values and perform lookup at read time.
    This will require circularity detection.

-   `src/DOM.cpp` will provide a memoized function to determine whether a DOM reference is valid.

-   `src/DOM.cpp` will provide a function to obtain a DOM reference value, with supporting metadata (type, writable).
