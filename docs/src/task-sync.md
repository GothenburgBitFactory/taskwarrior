# Synchronization

A single TaskChampion task database is known as a "replica".
A replica "synchronizes" its local information with other replicas via a sync server.
Many replicas can thus share the same task history.

This operation is triggered by running `task sync`.
Typically this runs frequently in a cron task.
Synchronization is quick, especially if no changes have occurred.

Each replica expects to be synchronized frequently, even if no server is involved.
Without periodic syncs, the storage space used for the task database will grow quickly, and performance will suffer.

By default, TaskChampion syncs to a "local server", as specified by the `server_dir` configuration parameter.
Every replica sharing a task history should have precisely the same configuration for `server_origin`, `server_client_key`, and `encryption_secret`.

Synchronizing a new replica to an existing task history is easy: begin with an empty replica, configured for the remote server, and run `task sync`.
The replica will download the entire task history.

It is possible to switch a single replica to a remote server by simply configuring for the remote server and running `task sync`.
The replica will upload the entire task history to the server.
Once this is complete, additional replicas can be configured with the same settings in order to share the task history.

