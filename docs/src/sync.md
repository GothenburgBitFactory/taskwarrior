# Synchronization and the Sync Server

This section covers *synchronization* of *replicas* containing the same set of tasks.
A replica is can perform all operations locally without connecting to a sync server, then share those operations with other replicas when it connects.
Sync is a critical feature of TaskChampion, allowing users to consult and update the same task list on multiple devices, without requiring constant connection.

This is a complex topic, and the section is broken into several chapters, beginning at the lower levels of the implementation and working up.
