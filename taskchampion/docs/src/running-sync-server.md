# Running the Sync Server

> NOTE: TaskChampion is still in development and not yet feature-complete.
> The server is functional, but lacks any administrative features.

Run `taskchampion-sync-server` to start the sync server.
Use `--port` to specify the port it should listen on, and `--data-dir` to specify the directory which it should store its data.
It only serves HTTP; the expectation is that a frontend proxy will be used for HTTPS support.

The server has optional parameters `--snapshot-days` and `--snapshot-version`, giving the target number of days and versions, respectively, between snapshots of the client state.
The default values for these parameters are generally adequate.
