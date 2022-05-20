# Task Database

The task database is a layer of abstraction above the replica storage layer, responsible for maintaining some important invariants.
While the storage is pluggable, there is only one implementation of the task database.

## Reading Data

The task database provides read access to the data in the replica's storage through a variety of methods on the struct.
Each read operation is executed in a transaction, so data may not be consistent between read operations.
In practice, this is not an issue for TaskChampion's purposes.

## Working Set

The task database maintains the working set.
The working set maps small integers to current tasks, for easy reference by command-line users.
This is done in such a way that the task numbers remain stable until the working set is rebuilt, at which point gaps in the numbering, such as for completed tasks, are removed by shifting all higher-numbered tasks downward.

The working set is not replicated, and is not considered a part of any consistency guarantees in the task database.

## Modifying Data

Modifications to the data set are made by applying operations.
Operations are described in [Replica Storage](./storage.md).

Each operation is added to the list of operations in the storage, and simultaneously applied to the tasks in that storage.
Operations are checked for validity as they are applied.

## Deletion and Expiration

Deletion of a task merely changes the task's status to "deleted", leaving it in the Task database.
Actual removal of tasks from the task database takes place as part of _expiration_, triggered by the user as part of a garbage-collection process.
Expiration removes tasks with a `modified` property more than 180 days in the past, by creating a `Delete(uuid)` operation.
