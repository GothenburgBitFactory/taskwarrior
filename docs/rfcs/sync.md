---
title: "Taskwarrior - Taskserver Sync Algorithm"
---


# Taskserver Sync Algorithm

This document describes how task changes are merged by the Taskserver.
It does not describe [the protocol](/docs/design/protocol) used by the Taskserver.

The Taskserver merges tasks from multiple sources, resulting in conflict- free syncing of data.
The algorithm used to achieve this is simple and effective, paralleling what SCM systems do to perform a rebase.


## Requirements

In this document, we adopt the convention discussed in Section 1.3.2 of
[RFC1122](https://tools.ietf.org/html/rfc1122#page-16) of using the capitalized words MUST, REQUIRED, SHOULD, RECOMMENDED, MAY, and OPTIONAL to define the significance of each particular requirement specified in this document.

In brief: "MUST" (or "REQUIRED") means that the item is an absolute requirement of the specification; "SHOULD" (or "RECOMMENDED") means there may exist valid reasons for ignoring this item, but the full implications should be understood before doing so; and "MAY" (or "OPTIONAL") means that this item is optional, and may be omitted without careful consideration.


## Problem Definition

The sync algorithm considers a single task, with multiple changes occurring in two separate locations that must be resolved.
The two locations are the local machine and the server.
This results in two parallel change sequences.

Examples using multiple clients collapse down to the simple two-branch case because the clients are merged serially.


## Change Sequence

A sequence of changes to the same task is represented as:

    T0 --> T1 --> T2

Although all examples are of the two-branch variety, some involve trivial branches.
Going through these examples will illustrate the algorithm.
First the legend:

    T0   Represents the original task, the base.
    T1   Represents the task with a non-trivial set of changes.
    T2   Represents the task with further changes.


## Deltas

The transition from T0 \--\> T1 can be seen as a transform applied to T0, resulting in T1.
That transform is the delta (d1) between T0 and T1, which is a subtractive term:

    d1 = (T1 - T0)

Therefore:

    T0 --> T1 = T0 + d1
              = T0 + (T1 - T0)

This states that the transition from T0 to T1 is the application of a delta to the original, T0, which results in T1.
Applying this to the whole change sequence yields:

    T0 --> T1 --> T2 = T0 + d1 + d2
                     = T0 + (T1 - T0) + (T2 - T1)


## Use Case Classification

Because clients sync requests are processed serially, there is no need to consider the multiple client cases.
This means there is only ever the case with two parallel change sequences = the two branch case.


## Two Branch Case

The two branch case represents changes made to the same task in two locations, resulting in two deltas that must be applied to the same base.

    T0 --> T1
    T0 --> T2

This reduces to a base with two deltas, but the order in which the deltas are applied is important.
For example:

    T0 + d1 + d2 =/= T0 + d2 + d1

The application of deltas is not commutative, except in the trivial case where the two deltas are identical, or the deltas do not overlap.
The deltas therefore need to be applied in the correct sequence.
Tasks have metadata that indicates the last modified time, which dictates the sequence.
Assuming d1 occurred before d2, this neatly collapses down to a single branch sequence:

    T0 + d1 + d2 = T3

Note that the result in this case is T3, because it will be neither T1 nor T2, unless the deltas are identical.


## Two Branch, Multiple Changes Case

The two branch case can be complicated by multiple changes per branch:

    T0 --> T1 --> T3 --> T5
    T0 --> T2 --> T4

Note that the numbers were chosen to represent the order in which the changes were made.
First a list of deltas is generated:

    T0 --> T1 = d1
    T1 --> T3 = d3
    T3 --> T5 = d5
    T0 --> T2 = d2
    T0 --> T4 = d4

    d1, d3, d5, d2, d4

Then the deltas are sorted by modified time:

    d1, d2, d3, d4, d5

Then epplied to the base, yielding T6:

    T0 + d1 + d2 + d3 + d4 +d5 = T6


## Two Branch Case Example

Suppose the base task looks like this:

    T0  project:ONE  due:tomorrow  priority:H  +tag1  Original description

The first branch looks like this:

    T1  project:TWO  due:23rd      priority:H  +tag1  Original description

The second branch looks like this:

    T2  project:ONE  due:tomorrow  priority:H  +tag1  Modified description

Delta d1 is:

    T0  project:ONE  due:tomorrow  priority:H  +tag1  Original description
    T1  project:TWO  due:23rd      priority:H  +tag1  Original description
    ----------------------------------------------------------------------
    d1  project:TWO  due:23rd

Delta d2 is:

    T0  project:ONE  due:tomorrow  priority:H  +tag1  Original description
    T2  project:ONE  due:tomorrow  priority:H  +tag1  Modified description
    ----------------------------------------------------------------------
    d2                                                Modified description

If d1 occurred before d2, the result is:

    T3 = T0 + d1 + d2
       = T0 + (project:TWO due:23rd) + (Modified description)

    T3 =  project:TWO  due:23rd  priority:H  +tag1  Modified description


## Use Cases

A range of illustrated use cases, from the trivial to the complex will show the algorithm in use.


## Use Case 1: New Local Task

Initial state:

    Server:  -
    Client:  T0

The server has no data, and so T0 is stored.
The result is now:

    Server:  T0
    Client:  T0


## Use Case 2: Local Change

Initial state:

    Server:  T0
    Client:  T0 --> T1

The server resolves the change:

    T0 --> T1 = T0 + d1
              = T1

T1 is stored.
The result is now:

    Server:  T0 --> T1
    Client:  T1


## Use Case 3: Local and Remote Change

Initial state:

    Server:  T0 --> T1
    Client:  T0 --> T2

This is the two branch case, and the deltas are generated:

    T0 --> T1 = T0 + d1
    T0 --> T2 = T0 + d2

The order of change is determine to be d1, d2, yielding T3:

    T3 = T0 + d1 + d2

T3 is stored on the server, and returned to the client.
The result is now:

    Server:  T0 --> T1 --> T2 --> T3
    Client:  T3


## Use Case 4: Multiple Local and Remote Changes

Initial state:

    Server:  T0 --> T1 --> T3
    Client:  T0 --> T2 --> T4

This is the two branch case, and the deltas are generated:

    T0 --> T1 = T0 + d1
    T1 --> T3 = T0 + d3
    T0 --> T2 = T0 + d2
    T2 --> T4 = T0 + d4

    d1, d3, d2, d4

The order of change is determine to be d1, d2, d3, d4, yielding T5:

    T5 = T0 + d1 + d2 + d3 + d4

T5 is stored on the server, and returned to the client.
The result is now:

    Server:  T0 --> T1 --> T2 --> T3 --> T4 --> T5
    Client:  T5
