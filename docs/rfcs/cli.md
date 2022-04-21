---
title: "Taskwarrior - Command Line Interface"
---

## Work in Progress

This design document is a work in progress, and subject to change. Once finalized, the feature will be scheduled for an upcoming release.


# CLI Syntax Update

The Taskwarrior command line syntax is being updated to allow more consistent and predictable results, while making room for new features.

Adding support for arbitrary expressions on the command line has become complicated because of the relaxed syntax of Taskwarrior. While the relaxed syntax allows for a very expressive command line, it also creates ambiguity for the parser, which needs to be reduced.

With some limited and careful changes it will be possible to have a clear and unambiguous command line syntax, which means a predictable and deterministic experience.

It should be stated that for straightforward and even current usage patterns, the command line will likely not change for you. Another goal is to not require changes to 3rd-party software, where possible. Only the more advanced and as-yet unintroduced features will require a more strict syntax. This is why now is an ideal time to tighten the requirements.


## Argument Types

The argument types supported remain the same, adding some new constructs.

* Config file override
  * `rc:<file>`

* Configuration override
  * `rc:<name>:<value>` Literal value
  * `rc:<name>=<value>` Literal value
  * `rc:<name>:=<value>` Calculated value

* Tag
  * `+<tag>`
  * `-<tag>`
  * `'+tag one'` Multi-word tag

* Attribute modifier
  * `rc:<name>.<modifier>:<value>`
  * Modifier is one of:
    * `before`
    * `after`
    * `under`
    * `over`
    * `above`
    * `below`
    * `none`
    * `any`
    * `is`
    * `isnt`
    * `equals`
    * `not`
    * `contains`
    * `has`
    * `hasnt`
    * `left`
    * `right`
    * `startswith`
    * `endswith`
    * `word`
    * `noword`

* Search pattern
  * `/<pattern>/`

* Substitution
  * `/<from>/<to>/`
  * `/<from>/<to>/g`

* Command
  * `add`
  * `done`
  * `delete`
  * `list`
  * etc.

* Separator
  * `--`

* ID Ranges
  * `<id>[-&ltid>][,<id>[-&ltid>]...]`

* UUID
  * `<uuid>`

* Everything Else
  * `<word>`
  * `'<word> <word> ...'`


## New Command Line Rules

Certain command line constructs will no longer be supported, and this is imposed by the new rules:

1.  Each command line argument may contain only one instance of one argument type, unless that type is `<word>`.

        task add project:Home +tag Repair the thing     # Good
        task add project:Home +tag 'Repair the thing'   # Good
        task add 'project:Home +tag Repair the thing'   # Bad

    Putting two arguments into one quoted arg makes that arg a `<word>`.

2.  If an argument type contains spaces, it must either be quoted or escaped.

        task add project:'Home & Garden' ...    # Good
        task add 'project:Home & Garden' ...    # Good
        task add project:Home\ \&\ Garden ...   # Good
        task add project:Home' & 'Garden ...    # Good
        task add project:Home \& Garden ...     # Bad

    The parser will not combine multiple arguments, for example:

        task '/one two/' list   # Good
        task /one two/ list     # Bad
        task /'one two'/ list   # Bad, unless ' is part of the pattern

3.  By default, *no* calculations are made, unless the `:=` eval operator is used, and if so, the whole argument may need to be quoted or escaped to satisfy Rule 1.

        task add project:3.project+x         # Literal
        task add project:=3.project+x        # DOM reference + concatenation

4.  Bare word search terms are no longer supported.
    Use the pattern type argument instead.

        task /foo/ list     # Good
        task foo list       # Bad

5.  Expressions must be a series of arguments, not a quoted string.

        task urgency \< 5.0 list     # Good
        task 'urgency < 5.0 list'    # Bad


## Other Changes

Aside from the command line parser, there are other changes needed:

-   Many online documents will need to be modified.

-   Filters will be automatically parenthesized, so that every command line will now looke like:

        task [overrides] [(cli-filter)] [(context-filter)] [(report-filter)] command [modifications]

-   There will be more errors when the command line is not understood.

-   Ambiguous ISO date formats are dropped.

        YYYYMMDD     # Bad
        YYYY-MM-DD   # Good

        hhmmss       # Bad
        hh:mm:ss     # Good

-   The tutorial videos will be even more out of date, and will be replaced by a large number of smaller demo 'movies'.
