# Replica Storage

Each replica has a storage backend.
The interface for this backend is given in `crate::taskstorage::Storage` and `StorageTxn`.

The storage is transaction-protected, with the expectation of a serializable isolation level.
The storage contains the following information:

- `tasks`: a set of tasks, indexed by UUID
- `base_version`: the number of the last version sync'd from the server (a single integer)
- `operations`: all operations performed since base_version
- `working_set`: a mapping from integer -> UUID, used to keep stable small-integer indexes into the tasks for users' convenience.  This data is not synchronized with the server and does not affect any consistency guarantees.

## Tasks

The tasks are stored as an un-ordered collection, keyed by task UUID.
Each task in the database has represented by a key-value map.
See [Tasks](./tasks.md) for details on the content of that map.

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
