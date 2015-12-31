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
#
# You can override some default options in your config.fish:
#
# # Tab-completion of task descriptions.
# # Warning: This often creates a list of suggestions which spans several pages,
# # and it usually pushes some of the commands and attributes to the end of the
# # list.
# set -g task_complete_task yes
#
# # Tab-completion of task IDs outside of the "depends" attribute.
# # Warning: This often creates a list of suggestions which spans several pages,
# # and it pushes all commands and attributes to the end of the list.
# set -g task_complete_id yes
#
# # Attribute modifiers (DEPRECATED since 2.4.0)
# set -g task_complete_attribute_modifiers yes
#
#
# Copyright 2014 - 2016, Roman Inflianskas <infroma@gmail.com>
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

# NOTE: remember that sed on OS X is different in some aspects. E.g. it does
#       not understand \t for tabs.

# convinience functions

function __fish.task.log
  for line in $argv
    echo $line >&2
  end
end

function __fish.task.partial
  set wrapped $argv[1]
  set what $argv[2]
  set -q argv[3]; and set f_argv $argv[3..-1]
  set f __fish.task.$wrapped.$what
  functions -q $f; and eval $f $f_argv
end

function __fish.task.zsh
  set -q argv[2]; and set task_argv $argv[2..-1]
  task _zsh$argv[1] $task_argv | sed 's/:/	/'
end


# command line state detection

function __fish.task.bare
  test (count (commandline -c -o)) -eq 1
end

function __fish.task.current.command
  # find command in commandline by list intersection
  begin; commandline -pco; and __fish.task.list._command all | cut -d '	' -f 1; end | sort | uniq -d | xargs
end

function __fish.task.before_command
  test -z (__fish.task.current.command)
end


# checking need to complete

function __fish.task.need_to_complete.attr_name
  __fish.task.need_to_complete.filter; or contains (__fish.task.current.command) (__fish.task.list.command_mods)
end

function __fish.task.need_to_complete.attr_value
  __fish.task.need_to_complete.attr_name
end

function __fish.task.need_to_complete.command
  switch $argv
    case all
      __fish.task.bare
    case filter
      __fish.task.before_command
  end
end

function __fish.task.need_to_complete.config
  contains (__fish.task.current.command) 'config' 'show'
end

function __fish.task.need_to_complete.filter
  __fish.task.before_command
end

function __fish.task.need_to_complete.id
  __fish.task.need_to_complete.filter
end

function __fish.task.need_to_complete.task
  __fish.task.need_to_complete.filter
end

function __fish.task.need_to_complete
  __fish.task.partial need_to_complete $argv
end


# list printers

function __fish.task.token_clean
  sed 's/[^a-z_.]//g; s/^\.$//g'
end

function __fish.task.list.attr_name
  task _columns | sed 's/$/:	attribute/g'
  # BUG: doesn't support file completion
  echo rc
end

function __fish.task.list.attr_value
  set token (commandline -ct | cut -d ':' -f 1 | cut -d '.' -f 1 | __fish.task.token_clean)
  if test -n $token
    set attr_names (__fish.task.list.attr_name | sed 's/:	/	/g' | grep '^'$token | cut -d '	' -f 1)
    for attr_name in $attr_names
      if test -n $attr_name
        __fish.task.list.attr_value_by_name $attr_name
      end
    end
  end
    __fish.task.list.tag
end

function __fish.task.list.attr_value_by_name
  set attr $argv[1]
  switch $attr
    case 'rc'
      __fish.task.list.rc
    case 'depends' 'limit' 'priority' 'status'
      __fish.task.combos_simple $attr (__fish.task.list $attr)
    # case 'description' 'due' 'entry' 'end' 'start' 'project' 'recur' 'until' 'wait'
    case '*'
      if [ "$task_complete_attribute_modifiers" = 'yes' ]; and echo (commandline -ct) | grep -q '\.'
        __fish.task.combos_with_mods $attr (__fish.task.list $attr)
      else
        __fish.task.combos_simple $attr (__fish.task.list $attr)
      end
  end
end

function __fish.task.list._command
  # Removed args until TW-1404 is fixed.
  #__fish.task.zsh commands $argv
  __fish.task.zsh commands
end

function __fish.task.list.command
  # ignore special commands
  __fish.task.list._command $argv | grep -Ev '^_'
end

function __fish.task.list.command_mods
  for command in 'add' 'annotate' 'append' 'delete' 'done' 'duplicate' 'log' 'modify' 'prepend' 'start' 'stop'
    echo $command
  end
end

function __fish.task.list.config
  task _config
end

function __fish.task.list.depends
  __fish.task.list.id with_description
end

function __fish.task.list.description
  __fish.task.zsh ids $argv | cut -d '	' -f 2-
end

function __fish.task.list.id
  set show_type $argv[1]
  if test -z $show_type
    task _ids
  else if [  $show_type = 'with_description' ]
    __fish.task.zsh ids
  end
end

# Attribure modifiers (DEPRECATED since 2.4.0)
function __fish.task.list.mod
  for mod in 'before' 'after' 'over' 'under' 'none' 'is' 'isnt' 'has' 'hasnt' 'startswith' 'endswith' 'word' 'noword'
    echo $mod
  end
end

function __fish.task.list.priority
  for priority in 'H' 'M' 'L'
    echo $priority
  end
end

function __fish.task.list.project
  task _projects
end

function __fish.task.list.rc
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
  __fish.task.zsh ids | sed -E 's/^(.*)	(.*)$/\2	task [id = \1]/g'
end

function __fish.task.list
  __fish.task.partial list $argv
end

function __fish.task.results_var_name
  echo $argv | sed 's/^/__fish.task.list /g; s/$/ results/g; s/[ .]/_/g;'
end

function __fish.task.list_results
  set var_name (__fish.task.results_var_name $name)
  for line in $$var_name
    echo $line
  end
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
    echo "$attr_name:"
  end
end

# Attribure modifiers (DEPRECATED since 2.4.0)
function __fish.task.combos_with_mods
  __fish.task.combos_simple $argv
  for mod in (__fish.task.list.mod)
    __fish.task.combos_simple $argv[1].$mod $argv[2..-1]
  end
end


# actual completion

function __fish.task.complete
  set what $argv
  set list_command "__fish.task.list $what"
  set check_function "__fish.task.need_to_complete $what"
  complete -c task -u -f -n $check_function -a "(eval $list_command)"
end

__fish.task.complete command all
__fish.task.complete command filter
__fish.task.complete attr_value
__fish.task.complete attr_name
__fish.task.complete config

if [ "$task_complete_task" = 'yes' ]
  __fish.task.complete task
end

if [ "$task_complete_id" = 'yes' ]
  __fish.task.complete id with_description
end
