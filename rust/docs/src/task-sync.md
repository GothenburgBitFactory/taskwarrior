# Synchronization

A single TaskChampion task database is known as a "replica".
A replica "synchronizes" its local information with other replicas via a sync server.
Many replicas can thus share the same task history.

This operation is triggered by running `ta sync`.
Typically this runs frequently in a cron task.
Synchronization is quick, especially if no changes have occurred.

Each replica expects to be synchronized frequently, even if no server is involved.
Without periodic syncs, the storage space used for the task database will grow quickly, and performance will suffer.

## Local Sync

By default, TaskChampion syncs to a "local server", as specified by the `server_dir` configuration parameter.
This defaults to `taskchampion-sync-server` in your [data directory](https://docs.rs/dirs-next/2.0.0/dirs_next/fn.data_dir.html), but can be customized in the configuration file.

## Remote Sync

For remote synchronization, you will need a few pieces of information.
From the server operator, you will need an origin and a client key.
Configure these with

```shell
ta config set server_origin "<origin from server operator>"
ta config set server_client_key "<client key from server operator>"
```

You will need to generate your own encryption secret.
This is used to encrypt your task history, so treat it as a password.
The following will use the `openssl` utility to generate a suitable value:

```shell
ta config set encryption_secret $(openssl rand -hex 35)
```

Every replica sharing a task history should have precisely the same configuration for `server_origin`, `server_client_key`, and `encryption_secret`.

### Adding a New Replica

Synchronizing a new replica to an existing task history is easy: begin with an empty replica, configured for the remote server, and run `ta sync`.
The replica will download the entire task history.

### Upgrading a Locally-Sync'd Replica

It is possible to switch a single replica to a remote server by simply configuring for the remote server and running `ta sync`.
The replica will upload the entire task history to the server.
Once this is complete, additional replicas can be configured with the same settings in order to share the task history.
