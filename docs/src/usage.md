# Usage

## `task`

The main interface to your tasks is the `task` command, which supports various subcommands.
You can find a quick list of all subcommands with `task help`.

Note that the `task` interface does not match that of TaskWarrior.

### Configuration

Temporarily, configuration is by environment variables.
The directory containing the replica's task data is given by `TASK_DB`, defaulting to `/tmp/tasks`.
the origin of the sync server is given by `SYNC_SERVER_ORIGIN`, defaulting to `http://localhost:8080`.
The client ID to use with the sync server is givne by `SYNC_SERVER_CLIENT_ID` (with no default).

## `taskchampion-sync-server`

Run `taskchampion-sync-server` to start the sync server.
It serves on port 8080 on all interfaces, using an in-memory database (meaning that all data is lost when the process exits).
Requests for previously-unknown clients are automatically added.
