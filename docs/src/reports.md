# Reports

As a to-do list manager, listing tasks is an important TaskChampion feature.
Reports are tabular displays of tasks, and allow very flexible filtering, sorting, and customization of columns.

TaskChampion includes several "built-in" reports, as well as supporting custom reports in the [configuration file](./config-file.md).

## Built-In Reports

The `next` report is the default, and lists all pending tasks:

```text
$ task
Id Description              Active Tags              
1  learn about TaskChampion        +next
2  buy wedding gift         *      +buy
```

The `Id` column contains short numeric IDs that are assigned to pending tasks.
These IDs are easy to type, such as to mark task 2 done (`task 2 done`).

The `list` report lists all tasks, with a similar set of columns.

## Custom Reports

Custom reports are defined in the configuration file's `reports` property.
This is a mapping from each report's name to its definition.
Each definition has the following properties:

* `filter` - criteria for the tasks to include in the report
* `sort` - how to order the tasks
* `columns` - the columns of information to display for each task

The filter is a list of filter arguments, just like those that can be used on the command line.
See the `task help` output for more details on this syntax.
For example:

```yaml
reports:
  garden:
    filter:
      - "status:pending"
      - "+garden"
```

The sort order is defined by an array of objects containing a `sort_by` property and an optional `ascending` property.
Tasks are compared by the first criterion, and if that is equal by the second, and so on.
For example:

```yaml
reports:
  garden:
    sort:
      - sort_by: description
      - sort_by: uuid
        ascending: false
```
If `ascending` is given, it can be `true` for the default sort order, or `false` for the reverse.

The available values of `sort_by` are

(TODO: generate automatically)

Finally, the configuration specifies the list of columns to display in the `columns` property.
Each element has a `label` and a `property`:

```yaml
reports:
    garden:
        columns:
          - label: Id
            property: id
          - label: Description
            property: description
          - label: Tags
            property: tags
```

The avaliable properties are:

(TODO: generate automatically)
