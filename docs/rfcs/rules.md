---
title: "Taskwarrior - Rule System"
---

## Work in Progress

This design document is a work in progress, and subject to change.
Once finalized, the feature will be scheduled for an upcoming release.


# Rule System

The rule system is a framework that supports highly configurable features, with runtime evaluation, DOM access and an internal API.
Implementing a rule system meets the goal of shrinking and stabilizing the product core, while adding new features, and enabling many more.


## Required Enhancements

To prepare for a Rules System, various subsystems must first be enhanced:

-   DOM references need to be unambiguous, and will all have the `dom.` prefix.

-   DOM references need to be able to access any Taskwarrior data, in any

-   Custom reports will change from referencing `<column>[.<format>]` to simply
    `<domref>`

-   RC file syntax needs to be enhanced, so support rule definitions, which are
    multi-line blocks that are indentation-sensitive

-   RC file syntax will support two ways of specifying the same data:

              a.b.c=...

              a:
                b:
                  c=...

-   RC file syntax will allow the use of environment variables inline:

              name=${TERM}
              include ${HOME}/.taskrc_local

-   The `Variant` object will migrate to `libshared`

-   The expression evaluator `Eval` object will migrate to `libshared`

-   The column objects will gain a more structured base class, and will serve as
    providers for DOM references

-   The 'exec' command will be able to run a rule, if the reference is correct

-   Taskwarrior will store state data in a new `state.data` file

-   `Config` object needs to use the `rat` parser, to tackle the more complex
    syntax

-   The RC file will support environment variable expansion, where `${NAME}`
    will be replaced by its corresponding value at launch time

At that point, the rules system can be implemented in `libshared`, and will use a pluggable architecture to allow its integration into several projects.

## DOM Enhancements

DOM references will be enhanced, with many more references supported.
All DOM references will begin with `dom.`, yielding unambiguous references.
References will have a type.
Types will support sub-references (`<date>.<month>`, `<tags>.<N>`, `<annotation>.<description>`), and display formats included.

       dom . [<id> .] <attribute> [. <sub-reference>] . <format>

       dom . 123   .  entry        . year             . yyyy
       dom . 123   .  entry
       dom . 123   .  tags
       dom . 123   .  tags         . count
       dom . 123   .  tags         . 1

In addition to direct attribute access, DOM references will also support tw references beyond the current set: dom.rc.<name>

       dom.cli.args
       dom.terminal.width
       dom.terminal.height
       dom.system.version
       dom.system.oѕ

And will also support higher-level constructs that do not directly correlate to attributes, for example:

       dom.active       Boolean indicator of any active tasks
       dom.synced       Boolean indicator of the need to sync
       dom.rc.path      String path of .taskrc file (or override)
       dom.data.path    String path of data directory
       dom.hooks.path   String path of hooks directory

Finally, access to state:

       dom.state.program
       dom.state.sync.last
       dom.state.sync.configured
       dom.state.run.last
       dom.state.context


## RC Syntax Changes

The current configuration system supports only two different forms of syntax:

    <name> = [ <value> ]

    include <file>

A rule is a new form of syntax that consists of the rule keyword, a name, optional trigger, followed by indented actions in the form of API calls and flow control.
For example:

    rule myRule() on_launch:
        # Some code here

A rule definition will appear in the RC file, alongside all the existing settings.
The rule syntax will require a blank line to terminate the rule definition, the result being that the RC file should be quite readable, although it will look like Python.


## Hook Scripts

While this functionality can also be implemented using hook scripts, rules will run in-process, and therefore do not require external interpreters to be launched every time.
This creates the potential to run faster than a hook script.

For complex processing, hook scripts will be the preferred mechanism, but as the rules system matures, rules will be made to run more quickly.
With adequate performance, a rule will be the preferred implementation over a hook script.
This is not expected to be the case at first.

Hook scripts are not likely to be extended beyond their current form, and with greater DOM access and a growing API, rules should be able to supplant most hook script use cases.


## Rule Triggers

The set of supported rule types will include:

* `on_launch` - Triggered on program launch.

* `on_add` - Triggered when a task is added.
  A context task will be provided.
  The rule can modify the task, and approve or reject it.

* `on_modify` - Triggered when a task is modified.
  A before and after context task will be provided.
  The rule can modify the task, and approve or reject it.

* `on_exit` - Triggered on program exit.

* `color` - Triggered when colors are being determined.

* `virtual tag` - Defines a new virtual tag.

* `format` - Triggered when an attribute needs formatting, defines are new format.

More rules types will be added for more capabilities in future releases.


## API

The API is a simple set of actions that may be taken by a rule.

* `debug(<string>)` - Displays the string in debug mode only and continues processing.

* `warn(<string>)` - Displays the string as a warning continues processing.

* `error(<string>)` - Displays the string as an error and terminates processing.

* `exec(<binary> [ <args> ... ])` - Executes the external program and passes arguments to it.
  If the program exits with non-zero status, it is treated as an error.

* `return <value>` - Provides a result value for the rule, when necessary.

This is a very limited set at first, and more API calls will be added to support capabilities in future releases.


## Grammar

The grammar closely tracks that of Python.
Blocks are indented consistently.

* `if <condition>: ... else: ...` - The condition is a full Algebraic expression, and supports none of the command line conveniences.
  Terms must be combined with logical operators.
  The condition is an expression that is evaluated and converted to a Boolean value.

* `for <name> in <collection>:` - There is no native type for a collection, but there are DOM references (`tags` \...) that reference collections.
  This provides a way to iterate.

* `set <name> = <expression>` - Writes to a named type.
  The name may be a writable DOM object (`dom...`) or temporary variable storage (`tmp...`).
  Writing to a read-only DOM reference is an error.

* `<function>([<args>])` - A function is either a rule or an API call.
  Calling an undefined function is an error.


## Examples

Here are some example rules which illustrate the syntax and API.

The replacement for the nag feature:

    rule Nag(before, after) on-modify:
      if before.urgency < tasks.max.urgency:
        warn ‘You have more urgent tasks’

      if after.status == 'completed' and before.urgency < (dom.urgency.max - 2.0):
        warn 'You have more urgent tasks!'

Correct commonly misspelled word:

    rule CorrectSpelling(task) on_add:
      set task.description = substitute(task.description, 'teh', 'the')

Abbreviation expansion:

    rule ExpandAbbreviation(task) on_modify:
      set task.description = substitute(task.description, '/TW-\d+/', 'https:\/\/github.com\/GothenburgBitFactory\/taskwarrior\/issues\/\1')

Warn on missing project:

    rule WarnOnMissingProject(task) on_add:
      if task.project == ‘’:
        warn(‘Project not specified’)

Color rule:

    rule ColorizeDue(task) color:
      if task.due > now:
        if task.due < (now + 5d):
          return dom.rc.color.due
        else:
          return dom.rc.color.due.later

Policy:

    rule policyProject(task) on_add:
        if task.project == '':
            if rc.default.project == '':
                error('You must specify a project')
            set task.project = rc.default.project
