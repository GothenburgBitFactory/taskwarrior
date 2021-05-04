# Reports

As a to-do list manager, listing tasks is an important TaskChampion feature.
Reports are tabular displays of tasks, and allow very flexible filtering, sorting, and customization of columns.

TaskChampion includes several "built-in" reports, as well as supporting custom reports in the [configuration file](./config-file.md).

## Built-In Reports

The `next` report is the default, and lists all pending tasks:

```text
$ ta
Id Description              Active Tags              
1  learn about TaskChampion        +next
2  buy wedding gift         *      +buy
3  plant tomatoes                  +garden
```

The `Id` column contains short numeric IDs that are assigned to pending tasks.
These IDs are easy to type, such as to mark task 2 done (`ta 2 done`).

The `list` report lists all tasks, with a similar set of columns.

## Custom Reports

Custom reports are defined in the configuration file's `reports` table.
This is a mapping from each report's name to its definition.
Each definition has the following properties:

* `filter` - criteria for the tasks to include in the report (optional)
* `sort` - how to order the tasks (optional)
* `columns` - the columns of information to display for each task

For example:

```toml
[reports.garden]
sort = [
    { sort_by = "description" }
]
filter = [
    "status:pending",
    "+garden"
]
columns = [
    { label = "ID", property = "id" },
    { label = "Description", property = "description" },
]
```

The filter is a list of filter arguments, just like those that can be used on the command line.
See the `ta help` output for more details on this syntax.
It will be merged with any filters provided on the command line, when the report is invoked.

The sort order is defined by an array of tables containing a `sort_by` property and an optional `ascending` property.
Tasks are compared by the first criterion, and if that is equal by the second, and so on.
If `ascending` is given, it can be `true` for the default sort order, or `false` for the reverse.

In most cases tasks are just sorted by one criterion, but a more advanced example might look like:

```toml
[reports.garden]
sort = [
    { sort_by = "description" }
    { sort_by = "uuid", ascending = false }
]
...
```

The available values of `sort_by` are

(TODO: generate automatically)

Finally, the `columns` configuration specifies the list of columns to display.
Each element has a `label` and a `property`, as shown in the example above.

The avaliable properties are:

(TODO: generate automatically)
