# Tasks

Tasks are stored internally as a key/value map with string keys and values.
All fields are optional: the `Create` operation creates an empty task.
Display layers should apply appropriate defaults where necessary.

## Atomicity

The synchronization process does not support read-modify-write operations.
For example, suppose tags are updated by reading a list of tags, adding a tag, and writing the result back.
This would be captured as an `Update` operation containing the amended list of tags.
Suppose two such `Update` operations are made in different replicas and must be reconciled:
 * `Update("d394be59-60e6-499e-b7e7-ca0142648409", "tags", "oldtag,newtag1", "2020-11-23T14:21:22Z")`
 * `Update("d394be59-60e6-499e-b7e7-ca0142648409", "tags", "oldtag,newtag2", "2020-11-23T15:08:57Z")`

The result of this reconciliation will be `oldtag,newtag2`, while the user almost certainly intended `oldtag,newtag1,newtag2`.

The key names given below avoid this issue, allowing user updates such as adding a tag or deleting a dependency to be represented in a single `Update` operation.

## Validity

_Any_ key/value map is a valid task.
Consumers of task data must make a best effort to interpret any map, even if it contains apparently contradictory information.
For example, a task with status "completed" but no "end" key present should be interpreted as completed at an unknown time.

## Representations

Integers are stored in decimal notation.

Timestamps are stored as UNIX epoch timestamps, in the form of an integer.

## Keys

The following keys, and key formats, are defined:

* `status` - one of `P` for a pending task (the default), `C` for completed or `D` for deleted
* `description` - the one-line summary of the task
* `modified` - the time of the last modification of this task
* `start` - the most recent time at which this task was started (a task with no `start` key is not active)
* `end` - if present, the time at which this task was completed or deleted (note that this key may not agree with `status`: it may be present for a pending task, or absent for a deleted or completed task)
* `tag_<tag>` - indicates this task has tag `<tag>` (value is an empty string)
* `wait` - indicates the time before which this task should be hidden, as it is not actionable
* `entry` - the time at which the task was created
* `annotation_<timestamp>` - value is an annotation created at the given time

The following are not yet implemented:

* `dep_<uuid>` - indicates this task depends on `<uuid>` (value is an empty string)

### UDAs

Any unrecognized keys are treated as "user-defined attributes" (UDAs).
These attributes can be used to store additional data associated with a task.
For example, applications that synchronize tasks with other systems such as calendars or team planning services might store unique identifiers for those systems as UDAs.
The application defining a UDA defines the format of the value.

UDAs _should_ have a namespaced structure of the form `<namespace>.<key>`, where `<namespace>` identifies the application defining the UDA.
For example, a service named "DevSync" synchronizing tasks from GitHub might use UDAs like `devsync.github.issue-id`.
Note that many existing UDAs for Taskwarrior integrations do not follow this pattern; these are referred to as legacy UDAs.
