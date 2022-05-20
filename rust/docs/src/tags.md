# Tags

Each task has a collection of associated tags.
Tags are short words that categorize tasks, typically written with a leading `+`, such as `+next` or `+jobsearch`.

Tags are useful for filtering tasks in reports or on the command line.
For example, when it's time to continue the job search, `ta +jobsearch` will show pending tasks with the `jobsearch` tag.

## Allowed Tags

Specifically, tags must be at least one character long and cannot contain whitespace or any of the characters `+-*/(<>^! %=~`.
The first character cannot be a digit, and `:` is not allowed after the first character.
All-capital tags are reserved for synthetic tags (below) and cannot be added or removed from tasks.

## Synthetic Tags

Synthetic tags are present on tasks that meet specific criteria, that are commonly used for filtering.
For example, `WAITING` is set for tasks that are currently waiting.
These tags cannot be added or removed from a task, but appear and disappear as the task changes.
The following synthetic tags are defined:

* `WAITING` - set if the task is waiting (has a `wait` property with a date in the future)
* `ACTIVE` - set if the task is active (has been started and not stopped)
* `PENDING` - set if the task is pending (not completed or deleted)
* `COMPLETED` - set if the task has been completed
* `DELETED` - set if the task has been deleted (but not yet flushed from the task list)
