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

Individual configuration parameters can be overridden by environemnt variables, converted to upper-case and prefixed with `TASKCHAMPION_`, e.g., `TASKCHAMPION_DATA_DIR`.
Nested configuration parameters cannot be overridden by environment variables.

The following configuration parameters are available:

* `data_dir` - path to a directory containing the replica's task data (which will be created if necessary).
  Default: `taskchampion` in the local data directory
* `server_origin` - Origin of the taskchampion sync server, e.g., `https://taskchampion.example.com`
* `server_client_id` -  Client ID to identify this replica to the sync server (a UUID)

## `taskchampion-sync-server`

Run `taskchampion-sync-server` to start the sync server.
It serves on port 8080 on all interfaces, using an in-memory database (meaning that all data is lost when the process exits).
Requests for previously-unknown clients automatically create the client.
