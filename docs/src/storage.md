# Replica Storage

Each replica has a storage backend.
The interface for this backend is given in `crate::taskstorage::TaskStorage` and `TaskStorageTxn`.

The storage is transaction-protected, with the expectation of a serializable isolation level.
The storage contains the following information:

- `tasks`: a set of tasks, indexed by UUID
- `base_version`: the number of the last version sync'd from the server
- `operations`: all operations performed since base_version
- `working_set`: a mapping from integer -> UUID, used to keep stable small-integer indexes into the tasks for users' convenience.  This data is not synchronized with the server and does not affect any consistency guarantees.

## Tasks

The tasks are stored as an un-ordered collection, keyed by task UUID.
Each task in the database has an arbitrary-sized set of key/value properties, with string values.

Tasks are only created and modified; "deleted" tasks continue to stick around and can be modified and even un-deleted.
Tasks have an expiration time, after which they may be purged from the database.

### Task Fields

Each task can have any of the following fields.
Timestamps are stored as UNIX epoch timestamps, in the form of an integer expressed in decimal notation.
Note that it is possible, in task storage, for any field to be omitted.

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

## Operations

Every change to the task database is captured as an operation.
In other words, operations act as deltas between database states.
Operations are crucial to synchronization of replicas, using a technique known as Operational Transforms.

Each operation has one of the forms 

 * `Create(uuid)`
 * `Delete(uuid)`
 * `Update(uuid, property, value, timestamp)`

The Create form creates a new task.
It is invalid to create a task that already exists.

Similarly, the Delete form deletes an existing task.
It is invalid to delete a task that does not exist.

The Update form updates the given property of the given task, where property and value are both strings.
Value can also be `None` to indicate deletion of a property.
It is invalid to update a task that does not exist.
The timestamp on updates serves as additional metadata and is used to resolve conflicts.
