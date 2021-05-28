# Environment Variables

## Configuration

Set `TASKCHAMPION_CONFIG` to the location of a configuration file in order to override the default location.

## Terminal Output

Taskchampion uses [termcolor](https://github.com/BurntSushi/termcolor) to color its output.
This library interprets [`TERM` and `NO_COLOR`](https://github.com/BurntSushi/termcolor#automatic-color-selection) to determine how it should behave, when writing to a tty.
Set `NO_COLOR` to any value to force plain-text output.

## Debugging

Both `ta` and `taskchampion-sync-server` use [env-logger](https://docs.rs/env_logger) and can be configured to log at various levels with the `RUST_LOG` environment variable.
For example:
```shell
$ RUST_LOG=taskchampion=trace ta add foo
```

The output may provide valuable clues in debugging problems.
