# Data Model

A client manages a single offline instance of a single user's task list.
The data model is only seen from the clients' perspective.

## Task Database

The task database is composed of an un-ordered collection of tasks, each keyed by a UUID.
Each task in the database has an arbitrary-sized set of key/value properties, with string values.

Tasks are only created and modified; "deleted" tasks continue to stick around and can be modified and even un-deleted.
Tasks have an expiration time, after which they may be purged from the database.

## Task Fields

Each task can have any of the following fields.
Timestamps are stored as UNIX epoch timestamps, in the form of an integer expressed in decimal notation.
Note that it is possible for any field to be omitted.

NOTE: This structure is based on https://taskwarrior.org/docs/design/task.html, but will diverge from that
model over time.

* `status` - one of `Pending`, `Completed`, `Deleted`, `Recurring`, or `Waiting`
* `entry` (timestamp) - time that the task was created
* `description` - the one-line summary of the task
* `start` (timestamp) - if set, the task is active and this field gives the time the task was started
* `end` (timestamp) - the time at which the task was deleted or completed
* `due` (timestamp) - the time at which the task is due
* `until` (timestamp) - the time after which recurrent child tasks should not be created
* `wait` (timestamp) - the time before which this task is considered waiting and should not be shown
* `modified` (timestamp) - time that the task was last modified
* `scheduled` (timestamp) - time that the task is available to start
* `recur` - recurrence frequency
* `mask` - recurrence history
* `imask` - for children of recurring tasks, the index into the `mask` property on the parent
* `parent` - for children of recurring tasks, the uuid of the parent task
* `project` - the task's project (usually a short identifier)
* `priority` - the task's priority, one of `L`, `M`, or `H`.
* `depends` - a comma (`,`) separated list of uuids of tasks on which this task depends
* `tags` - a comma (`,`) separated list of tags for this task
* `annotation_<timestamp>` - an annotation for this task, with the timestamp as part of the key
* `udas` - user-defined attributes
