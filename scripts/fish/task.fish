# Taskwarrior completions for the Fish shell <http://fishshell.org>
#
# taskwarrior - a command line task list manager.
#
# Completions should work out of box. If it isn't, fill the bug report on your
# operation system bug tracker.
#
# As a workaround you can copy this script to
# ~/.config/fish/completions/task.fish, and open a new shell.
#
# Objects completed:
#  * Commands
#  * Projects
#  * Priorities
#  * Tags
#  * Attribute names and modifiers
#
# Copyright 2014 Roman Inflianskas <infroma@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# http://www.opensource.org/licenses/mit-license.php


# convinience functions

function __fish.task.log
  for line in $argv
    echo $line >&2
  end
end

function __fish.task.partial
  set wrapped $argv[1]
  set what $argv[2]
  set -q argv[3]; and set argv $argv[3..-1]
  set f __fish.task.$wrapped.$what
  functions -q $f; and eval $f $argv
end


# command line state detection

function __fish.task.bare
    test (count (commandline -c -o)) -eq 1
end

function __fish.task.current.command
  # find command in commandline by list intersection
  begin; commandline -pco; and __fish.task.list._command all | cut -d ':' -f 1; end | sort | uniq -d | xargs
end

function __fish.task.before_command
  test -z (__fish.task.current.command)
end


# checking need to complete

function __fish.task.need_to_complete.attr_name
  __fish.task.need_to_complete.filter; or contains (__fish.task.current.command) (__fish.task.list.command_mods)
end

function __fish.task.need_to_complete.attr_value
  __fish.task.need_to_complete.filter
end

function __fish.task.need_to_complete.command
  switch $argv
    case all
      __fish.task.bare
    case filter
      __fish.task.before_command
  end
end

function __fish.task.need_to_complete.filter
  __fish.task.before_command
end

function __fish.task.need_to_complete.id
  __fish.task.need_to_complete.filter
end

function __fish.task.need_to_complete
  __fish.task.partial need_to_complete $argv
end


# list printers

function __fish.task.list.attr_name
  task _columns | sed 's/$/:/g'
  echo rc
end

function __fish.task.list._command
  task _zshcommands $argv
end

function __fish.task.list.command
  # ignore special commands
  __fish.task.list._command $argv | grep -Ev '^_'
end

function __fish.task.list.command_mods
  # BUG: fill issue to have _zshcommand mods
  for command in 'add' 'annotate' 'append' 'delete' 'done' 'duplicate' 'log' 'modify' 'prepend' 'start' 'stop'
    echo $command
  end
end

function __fish.task.list.depends
  task _ids
end

function __fish.task.list.description
  task _zshids | cut -d ':' -f 2
end

function __fish.task.list.id
  task _zshids
end

function __fish.task.list.mod
  # BUG: remove in 2.4.0
    echo before
    echo after
    echo over
    echo under
    echo none
    echo is
    echo isnt
    echo has
    echo hasnt
    echo startswith
    echo endswith
    echo word
    echo noword
end

function __fish.task.list.priority
  echo H
  echo M
  echo L
end

function __fish.task.list.project
    task _projects
end

function __fish.task.list.rc
    echo rc:
    for value in (task _config)
        echo rc.$value:
    end
end

function __fish.task.list.status
  echo pending
  echo completed
  echo deleted
  echo waiting
end

function __fish.task.list.tag
    for tag in (task _tags)
        echo +$tag
        echo -$tag
    end
end

function __fish.task.list.task
  task _zshids | sed -E 's/^(.*):(.*)$/"\2":\1/g'
end

function __fish.task.list
  __fish.task.partial list $argv
end


# working with attributes

function __fish.task.combos_simple
  set attr_name $argv[1]
  set -q argv[2]; and set attr_values $argv[2..-1]
  if [ (count $attr_values) -gt 0 ]
    for attr_value in $attr_values
      echo "$attr_name:$attr_value"
    end
  else
    echo $attr_name:
  end
end

function __fish.task.combos_with_mods
  # BUG: remove in 2.4.0
  __fish.task.combos_simple $argv
    for mod in (__fish.task.list.mod)
        __fish.task.combos_simple $argv[1].$mod $argv[2..-1]
    end
end

function __fish.task.list.attr_value
  set attr $argv[1]
  switch $attr
    case 'depends' 'limit' 'priority' 'rc' 'status'
      __fish.task.combos_simple $attr (__fish.task.list $attr)
    # case 'description' 'due' 'entry' 'end' 'start' 'project' 'recur' 'until' 'wait'
    case '*'
      # BUG: remove in 2.4.0
      if echo (commandline -ct) | grep -q '\.'
        __fish.task.combos_with_mods $attr (__fish.task.list $attr)
      else
        __fish.task.combos_simple $attr (__fish.task.list $attr)
      end
  end
end

function __fish.task.body
    set token (commandline -ct)

    if test -n $token
        __fish.task.attr $token
    end

    __fish.task.list.tag
end


# actual completion

function __fish.task.complete
    complete -c task -A -f $argv
end

function __fish.task.complete.with_description_2
    # argv: list_command check_function
  for arg_description in (eval $argv[1])
    set arg         (echo $arg_description | cut -d ':' -f 1)
    set description (echo $arg_description | cut -d ':' -f 2)
    __fish.task.complete -n $argv[2] -a $arg -d $description
  end
end

function __fish.task.complete.with_description
  __fish.task.complete.with_description_2 "__fish.task.list $argv" "__fish.task.need_to_complete $argv"
end

function __fish.task.list.current_attr_value
  set token (commandline -ct | cut -d ':' -f 1 | cut -d '.' -f 1 | sed 's/[^a-z_.]//g; s/^\.$//g')
  if test -n $token
    set attr_names (__fish.task.list.attr_name | grep '^'$token | cut -d ':' -f 1)
    for attr_name in $attr_names
      if test -n $attr_name
        __fish.task.list.attr_value $attr_name
      end
    end
  end
    __fish.task.list.tag
end

__fish.task.complete.with_description command all
__fish.task.complete.with_description command filter
__fish.task.complete.with_description id
__fish.task.complete.with_description attr_name
__fish.task.complete -n '__fish.task.need_to_complete.attr_name' -a '(__fish.task.list.current_attr_value)'

