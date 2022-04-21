---
title: "Taskwarrior - Work Week Support"
---

## Work in Progress

This design document is a work in progress, and subject to change.
Once finalized, the feature will be scheduled for an upcoming release.


# Work Week Support

Taskwarrior supports the idea that a week starts on either a Sunday or a Monday, as determined by configuration.
This was added eight years ago, simply for display purposes in the `calendar` report.
Since then its use has propagated and it influences the `sow` date reference.0

Further requests have been made to make this more flexible, so that the notion of 'weekend' can be defined.
Furthermore, the idea that every week has a weekend has also been questioned.

It has become clear that a `weekstart` setting, and the notion of a weekend are no longer useful.

## Proposed Support

One option is to allow the user to completely define a work week in the following way:

    workweek=1,2,3,4,5

With Sunday as day zero, this states that the work week is the typical Monday - Friday.
From this setting, the meaning of `soww` and `eoww` can be determined, as well as `recur:weekday`.
