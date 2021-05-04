# Debugging

Both `ta` and `taskchampion-sync-server` use [env-logger](https://docs.rs/env_logger) and can be configured to log at various levels with the `RUST_LOG` environment variable.
For example:
```shell
$ RUST_LOG=taskchampion=trace ta add foo
```

The output may provide valuable clues in debugging problems.
