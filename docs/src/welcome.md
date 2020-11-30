# TaskChampion

TaskChampion is a personal task-tracking tool.
It works from the command line, with simple commands like `task add "fix the kitchen sink"`.
It can synchronize tasks on multiple devices, and does so in an "offline" mode so you can update your tasks even when you can't reach the server.
If you've heard of [TaskWarrior](https://taskwarrior.org/), this tool is very similar, but actively maintained and with a more reliable synchronization system.

## Getting Started

> NOTE: TaskChampion is still in development and not yet feature-complete.
> This section is limited to completed functionality.

Once you've [installed TaskChampion](./installation.md), your interface will be via the `task` command.
Start by adding a task:

```shell
$ task add "learn how to use taskchampion"
added task ba57deaf-f97b-4e9c-b9ab-04bc1ecb22b8
```

You can see all of your pending tasks with `task pending`, or just `task` for short:

```shell
$ task
 id act description                   
 1      learn how to use taskchampion
```

Tell TaskChampion you're working on the task, using the shorthand id:

```shell
$ task start 1
```

and when you're done with the task, mark it as complete:

```shell
$ task done 1
```

## Synchronizing

Even if you don't have a server, it's a good idea to sync your task database periodically.
This acts as a backup and also enables some internal house-cleaning.

```shell
$ task sync
```

Typically sync is run from a crontab, on whatever schedule fits your needs.

To synchronize multiple replicas of your tasks, you will need a sync server and a client ID on that server.
Configure these in `~/.config/taskchampion.yml`, for example:

```yaml
server_client_id: "f8d4d09d-f6c7-4dd2-ab50-634ed20a3ff2"
server_origin: "https://taskchampion.example.com"
```

The next run of `task sync` will upload your task history to that server.
Configuring another device identically and running `task sync` will download that task history, and continue to stay in sync with subsequent runs of the command.
