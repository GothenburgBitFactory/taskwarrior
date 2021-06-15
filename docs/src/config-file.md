# Configuration

The `ta` command will work out-of-the-box with no configuration file, using default values.

Configuration is read from `taskchampion.toml` in your config directory.
On Linux systems, that directory is `~/.config`.
On OS X, it's `~/Library/Preferences`.
On Windows, it's `AppData/Roaming` in your home directory.
This can be overridden by setting `TASKCHAMPION_CONFIG` to the configuration filename.

The file format is [TOML](https://toml.io/).
For example:

```toml
data_dir = "/home/myuser/.tasks"
```

## Directories

* `data_dir` - path to a directory containing the replica's task data (which will be created if necessary).
  Default: `taskchampion` in the local data directory.

## Command-Line Preferences

* `modification_count_prompt` - when a modification will affect more than this many tasks, the `ta` command will prompt for confirmation.
  A value of `0` will disable the prompts entirely.
  Default: 3.

## Sync Server

If using a local server:

* `server_dir` - path to a directory containing the local server's data.
  This is only used if `server_origin` or `server_client_key` are not set.
  Default: `taskchampion-sync-server` in the local data directory.

If using a remote server:

* `server_origin` - Origin of the TaskChampion sync server, e.g., `https://taskchampion.example.com`.
  If not set, then sync is done to a local server.
* `encryption_secret` - Secret value used to encrypt all data stored on the server.
  This should be a long random string.
  If you have `openssl` installed, a command like `openssl rand -hex 35` will generate a suitable value.
  This value is only used when synchronizing with a remote server -- local servers are unencrypted.
  Treat this value as a password.
* `server_client_key` -  Client key to identify this replica to the sync server (a UUID)
  If not set, then sync is done to a local server.

## Reports

* `reports` - a mapping of each report's name to its definition.
  See [Reports](./reports.md) for details.

## Editing

As a shortcut, the simple, top-level configuration values can be edited from the command line:

```shell
ta config set data_dir /home/myuser/.taskchampion
```
