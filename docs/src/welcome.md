# TaskChampion

TaskChampion is a personal task-tracking tool.
It works from the command line, with simple commands like `ta add "fix the kitchen sink"`.
It can synchronize tasks on multiple devices, and does so in an "offline" mode so you can update your tasks even when you can't reach the server.
If you've heard of [TaskWarrior](https://taskwarrior.org/), this tool is very similar, but with some different design choices and greater reliability.

## Getting Started

> NOTE: TaskChampion is still in development and not yet feature-complete.
> This section is limited to completed functionality.

Once you've [installed TaskChampion](./installation.md), your interface will be via the `ta` command.
Start by adding a task:

```shell
$ ta add learn how to use taskchampion
added task ba57deaf-f97b-4e9c-b9ab-04bc1ecb22b8
```

You can see all of your pending tasks with `ta next`, or just `ta` for short:

```shell
$ ta
 Id Description                    Active  Tags
 1  learn how to use taskchampion
```

Tell TaskChampion you're working on the task, using the shorthand id:

```shell
$ ta start 1
```

and when you're done with the task, mark it as complete:

```shell
$ ta done 1
```

## Synchronizing

Even if you don't have a server, it's a good idea to sync your task database periodically.
This acts as a backup and also enables some internal house-cleaning.

```shell
$ ta sync
```

Typically sync is run from a crontab, on whatever schedule fits your needs.

To synchronize multiple replicas of your tasks, you will need a sync server and a client key for that server.
Configure these in `~/.config/taskchampion.yml`, for example:

```yaml
server_client_key: "f8d4d09d-f6c7-4dd2-ab50-634ed20a3ff2"
server_origin: "https://taskchampion.example.com"
```

The next run of `ta sync` will upload your task history to that server.
Configuring another device identically and running `ta sync` will download that task history, and continue to stay in sync with subsequent runs of the command.

See [Usage](./using-task-command.md) for more detailed information on using TaskChampion.
