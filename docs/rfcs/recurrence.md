---
title: "Taskwarrior - Recurrence"
---

# Draft

This is a draft design document.
Your [feedback](mailto:support@taskwarrior.org?Subject=Feedback) is welcomed.

Recurrence
----------

Recurrence needs an overhaul to improve weaknesses and add new features.

# Terminology

-   The hidden 'parent' task is called the template.
-   Synthesis is the name for the generation of new recurring task instances when necessary.
-   The synthesized tasks are called instances.
-   The index is the zero-based monotonically increasing number of the instance.
-   Drift is the accumulated errors in time that cause a due date to slowly change for each recurring task.

# Criticism of Current Implementation

-   The `mask` attribute grows unbounded.
-   Only strict recurrence cycles are supported.
    The example of mowing the lawn is that you want to mow the lawn every seven days, but when you are four days late mowing the lawn, the next mowing should be in seven days, not in three.
-   Intances generated on one machine and then synced, may collide with equivalent unsynced instances tasks on another device, because the UUIDs are different.
-   You cannot `wait` a recurring task and have that wait period propagate to all other child tasks.
-   Task instances cannot individually expire.

# Proposals

## Proposal: Eliminate `mask`, `imaѕk` Attributes

The `mask` attribute in the template is replaced by `last`, which indicates the most recent instance index synthesized.
Because instances are never synthesized out of order, we only need to store the most recent index.
The `imask` attribute in the instance is no longer needed.

## Proposal: Rename `parent` to `template`

The name `parent` implies subtasks, and confuses those who inspect the internals.
The value remains the UUID of the template.
This frees up the namespace for future use with subtasks.

## Proposal: New 'rtype' attribute

To indicate the flavor of recurrence, support the following values:

* `periodic` - Instances are created on a regular schedule.
  Example: send birthday flowers.
  It must occur on a regular schedule, and doesn't matter if you were late last year.
  This is the default value.

* `chained` - Instances are created back to back, so when one instance ends, the next begins, with the same recurrence.
  Example: mow the lawn.
  If you mow two days late, the next instance is not two days early to compensate.

## Proposal: Use relative offsets

The delta between `wait` and `due` date in the template should be reflected in the delta between `wait` and `due` date in the instance.
Similarly, 'scheduled' must be handled the same way.

## Proposal: On load, auto-upgrade legacy tasks

Upgrade template:

-   Add `rtype:periodic`
-   Add `last:N` where `N` is the length of `mask`
-   Delete `mask`

Upgrade instance:

-   Rename `parent` to `template`
-   Delete `imask`
-   Update `wait` if not set to: `wait:due + (template.due - template.wait)`
-   Update `scheduled` if not set to: `scheduled:due + (template.due - template.scheduled)`

## Proposal: Deleting a chained instance

Deleting a `rtype:chained` instance causes the next chained instance to be synthesized.
This gives the illusion that the due date is simply pushed out to `(now + template.recur)`.

## Proposal: Modification Propagation

TBD

## Proposal: Exotic Dates

Expand date specifications to use pattern phrases:

-   `4th thursday in November`
-   `4th thursday of November`
-   `Friday before easter`
-   `next Tuesday`
-   `last Tuesday`
-   `last July`
-   `weekend`
-   `3 days before eom`
-   `in the morning`
-   `4pm`
-   `noon`
-   `midnight`

Got suggestions?

## Proposal: User-Defined Week Start

TBD

# Implementation

## Implementation: Adding a new `periodic` template

When adding a new periodic template:

    task add ... due:D recur:R wait:D-1wk scheduled:D-1wk until:U

Creates:

    template.uuid:        NEW_UUID
    template.description: ...
    template.entry:       now
    template.modified:    now
    template.due:         D
    template.recur:       R       (stored in raw form, ie 'P14D')
    template.wait:        D-1wk
    template.scheduled:   D-1wk
    template.until:       U
    template.rtype:       periodic
    template.last:

Creating the Nth instance (index N):

    Clone instance from template.

    instance.uuid:        NEW_UUID
    instance.modified:    now
    instance.due:         template.due + (N * template.recur)
    instance.wait:        instance.due + (template.due - template.wait)
    instance.scheduled:   instance.due + (template.due - template.scheduled)
    instance.start:

    template.last:        N

## Implementation: Adding a new `chained` template

When adding a new chained template:

    task add ... due:D recur:R wait:D-1wk scheduled:D-1wk until:U rtype:chained

Creates:

    template.uuid:        NEW_UUID
    template.description: ...
    template.entry:       now
    template.modified:    now
    template.due:         D
    template.recur:       R       (stored in raw form, ie 'P14D')
    template.wait:        D-1wk
    template.scheduled:   D-1wk
    template.until:       U
    template.rtype:       chained

Creating the Nth instance (index N):

    Clone instance from template.

    instance.uui  d:        NEW_UUID
    instance.mod  ified:    now
    instance.due  :         instance[N-1].end + template.recur
    instance.wai  t:        instance.due + (template.due - template.wait)
    instance.sch  eduled:   instance.due + (template.due - template.scheduled)
    instance.sta  rt:

Chained tasks do not obey `rc.recurrence.limit`, and show only one pending task
at a time.

## Implementation: Special handling for months

Certain recurrence periods are inexact:

-   P1M
-   P1Y
-   P1D

When the recurrence period is `P1M` the number of days in a month varies and causes drift.

When the recurrence period is `P1Y` the number of days in a year varies and causes drift.

When the recurrence period is `P1D` the number of hours in a day varies due to daylight savings, and causes drift.

Drift should be avoided by carefully implementing:

    instance.due: template.due + (N * template.recur)
