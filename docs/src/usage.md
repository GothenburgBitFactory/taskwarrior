# Usage

## `task`

The main interface to your tasks is the `task` command, which supports various subcommands.
You can find a quick list of all subcommands with `task help`.

Note that the `task` interface does not match that of TaskWarrior.

### Configuration

The `task` command will work out-of-the-box with no configuration file, using default values.

Configuration is read from `taskchampion.yaml` in your config directory.
On Linux systems, that directory is `~/.config`.
On OS X, it's `~/Library/Preferences`.
On Windows, it's `AppData/Roaming` in your home directory.
The path can be overridden by setting `$TASKCHAMPION_CONFIG`.

Individual configuration parameters can be overridden by environment variables, converted to upper-case and prefixed with `TASKCHAMPION_`, e.g., `TASKCHAMPION_DATA_DIR`.
Nested configuration parameters cannot be overridden by environment variables.

The following configuration parameters are available:

* `data_dir` - path to a directory containing the replica's task data (which will be created if necessary).
  Default: `taskchampion` in the local data directory.
* `server_dir` - path to a directory containing the local server's data.
  This is only used if `server_origin` or `server_client_id` are not set.
  Default: `taskchampion-sync-server` in the local data directory.
* `encryption_secret` - Secret value used to encrypt all data stored on the server.
  This should be a long random string.
  If you have `openssl` installed, a command like `openssl rand -hex 35` will generate a suitable value.
  This value is only used when synchronizing with a remote server -- local servers are unencrypted.
  Treat this value as a password.
* `server_origin` - Origin of the TaskChampion sync server, e.g., `https://taskchampion.example.com`.
  If not set, then sync is done to a local server.
* `server_client_id` -  Client ID to identify this replica to the sync server (a UUID)
  If not set, then sync is done to a local server.

### Synchronization

A single TaskChampion task database is known as a "replica".
A replica "synchronizes" its local information with other replicas via a sync server.
Many replicas can thus share the same task history.

This operation is triggered by running `task sync`.
Typically this runs frequently in a cron task.
Synchronization is quick, especially if no changes have occurred.

Each replica expects to be synchronized frequently, even if no server is involved.
Without periodic syncs, the storage space used for the task database will grow quickly, and performance will suffer.

By default, TaskChampion syncs to a "local server", as specified by the `server_dir` configuration parameter.
Every replica sharing a task history should have precisely the same configuration for `server_origin`, `server_client_id`, and `encryption_secret`.

Synchronizing a new replica to an existing task history is easy: begin with an empty replica, configured for the remote server, and run `task sync`.
The replica will download the entire task history.

It is possible to switch a single replica to a remote server by simply configuring for the remote server and running `task sync`.
The replica will upload the entire task history to the server.
Once this is complete, additional replicas can be configured with the same settings in order to share the task history.

## `taskchampion-sync-server`

Run `taskchampion-sync-server` to start the sync server.
Use `--port` to specify the port it should listen on, and `--data-dir` to specify the directory which it should store its data.
It only serves HTTP; the expectation is that a frontend proxy will be used for HTTPS support.

## Debugging

Both `task` and `taskchampio-sync-server` use [env-logger](https://docs.rs/env_logger) and can be configured to log at various levels with the `RUST_LOG` environment variable.
For example:
```shell
$ RUST_LOG=taskchampion=trace task add foo
```
