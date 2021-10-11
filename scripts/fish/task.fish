# Taskwarrior completions for the Fish shell <https://fishshell.com>
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
# Copyright 2014 - 2021, Roman Inflianskas <infroma@gmail.com>
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
# https://www.opensource.org/licenses/mit-license.php

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
  begin; commandline -pco; and echo $__fish_task_static_commands; end | sort | uniq -d | xargs
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
  or return 1
  # only start completion when there's a colon in attr_name
  set -l cmd (commandline -ct)
  string match -q -- "*:*" "$cmd[-1]"
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

function __fish.task.need_to_complete.tag
  __fish.task.need_to_complete.attr_name
  or return 1
  set -l cmd (commandline -ct)
  # only start complete when supplied + or -
  string match -qr -- "^[+-][^+-]*" "$cmd[-1]"
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
  # # BUG: doesn't support file completion
  for attr in (task _columns)
    if set -l idx (contains -i -- $attr $__fish_task_static_attr_desc_keys)
      # use builtin friendly description
      echo -e "$attr:\tattribute:$__fish_task_static_attr_desc_vals[$idx]"
    else
      echo -e "$attr:\tattribute"
    end
  end
  echo -e "rc\tConfiguration for taskwarrior"
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
end

function __fish.task.list.attr_value_by_name
  set attr $argv[1]
  switch $attr
    case 'rc'
      __fish.task.list.rc
    case 'depends' 'limit' 'priority' 'status'
      __fish.task.combos_simple $attr (__fish.task.list $attr)
    case 'recur'
      __fish.task.combos_simple $attr (__fish.task.list.date_freq)
    case 'due' 'until' 'wait' 'entry' 'end' 'start' 'scheduled'
      __fish.task.combos_simple $attr (__fish.task.list.dates)
    # case 'description' 'project'
    case '*'
      if [ "$task_complete_attribute_modifiers" = 'yes' ]; and echo (commandline -ct) | grep -q '\.'
        __fish.task.combos_with_mods $attr (__fish.task.list $attr)
      else
        __fish.task.combos_simple $attr (__fish.task.list $attr)
      end
  end
end

function __fish.task.list._command
  echo -e $__fish_task_static_commands_with_desc
end

function __fish.task.list.command
  # ignore special commands
  __fish.task.list._command $argv | command grep -Ev '^_'
end

function __fish.task.list.command_mods
  echo -e $__fish_task_static_command_mods
end

function __fish.task.list.config
  task _config
end

function __fish.task.list.depends
  __fish.task.list.id with_description
end

function __fish.task.list.description
  __fish.task.zsh ids $argv | awk -F"\t" '{print $2 "\tid=" $1}'
end

function __fish.task.list.id
  set show_type $argv[1]
  if test -z $show_type
    task _ids
  else if [ $show_type = 'with_description' ]
    __fish.task.zsh ids
  end
end

function __fish.task.list.date_freq
  set -l cmd (commandline -ct)
  if set -l user_input_numeric (echo $cmd[-1] | grep -o '[0-9]\+')
    # show numeric freq like 2d, 4m, etc.
    echo -e (string replace --all -r "^|\n" "\n$user_input_numeric" $__fish_task_static_freq_numeric | string collect)
  else
    echo -e $__fish_task_static_freq
  end
end

function __fish.task.list.dates
  set -l cmd (commandline -ct)
  if set -l user_input_numeric (echo $cmd[-1] | grep -o '[0-9]\+')
    # show numeric date like 2hrs, 4th, etc.
    echo -e (string replace --all -r "^|\n" "\n$user_input_numeric" $__fish_task_static_reldates | string collect)
    # special cases for 1st, 2nd and 3rd, and 4-0th
    set -l suffix 'th' '4th, 5th, etc.'
    if string match -q -- "*1" $user_input_numeric
      set suffix 'st' 'first'
    else if string match -q -- "*2" $user_input_numeric
      set suffix 'nd' 'second'
    else if string match -q -- "*3" $user_input_numeric
      set suffix 'rd' 'third'
    end
    echo -e $user_input_numeric"$suffix[1]\t$suffix[2]"
  else
    echo -e $__fish_task_static_dates
  end
end

# Attribure modifiers (DEPRECATED since 2.4.0)
function __fish.task.list.mod
  echo -e $__fish_task_static_mod
end

function __fish.task.list.priority
  echo -e $__fish_task_static_priority
end

function __fish.task.list.project
  task _projects
end

function __fish.task.list.rc
  task _config
end

function __fish.task.list.status
  echo -e $__fish_task_static_status
end

function __fish.task.list.tag
  set -l tags (task _tags)
  printf -- '+%s\n' $tags
  printf -- '-%s\n' $tags
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
  complete -c task -u -k -f -n $check_function -a "(eval $list_command)"
end

# static variables that won't changes even when taskw's data is modified
set __fish_task_static_commands_with_desc (__fish.task.zsh commands | sort | string collect)
set __fish_task_static_commands (echo -e $__fish_task_static_commands_with_desc | cut -d '	' -f 1 | string collect)
set __fish_task_static_command_mods (printf -- '%s\n' 'add' 'annotate' 'append' 'delete' 'done' 'duplicate' 'log' 'modify' 'prepend' 'start' 'stop' | string collect)
set __fish_task_static_mod (printf -- '%s\n' 'before' 'after' 'over' 'under' 'none' 'is' 'isnt' 'has' 'hasnt' 'startswith' 'endswith' 'word' 'noword' | string collect)
set __fish_task_static_status (printf -- '%s\tstatus\n' 'pending' 'completed' 'deleted' 'waiting' | string collect)
set __fish_task_static_priority (printf -- '%s\n' 'H\tHigh' 'M\tMiddle' 'L\tLow' | string collect)

set __fish_task_static_freq 'daily:Every day' \
                            'day:Every day' \
                            'weekdays:Every day skipping weekend days' \
                            'weekly:Every week' \
                            'biweekly:Every two weeks' \
                            'fortnight:Every two weeks' \
                            'monthly:Every month' \
                            'quarterly:Every three months' \
                            'semiannual:Every six months' \
                            'annual:Every year' \
                            'yearly:Every year' \
                            'biannual:Every two years' \
                            'biyearly:Every two years'
set __fish_task_static_freq (printf -- '%s\n' $__fish_task_static_freq | sed 's/:/\t/' | string collect)
set __fish_task_static_freq_numeric 'd:days' \
                                    'w:weeks' \
                                    'q:quarters' \
                                    'y:years'
set __fish_task_static_freq_numeric (printf -- '%s\n' $__fish_task_static_freq_numeric | sed 's/:/\t/' | string collect)
set __fish_task_static_freq_numeric 'd:days' \
                                    'w:weeks' \
                                    'q:quarters' \
                                    'y:years'
set __fish_task_static_freq_numeric (printf -- '%s\n' $__fish_task_static_freq_numeric | sed 's/:/\t/' | string collect)
set __fish_task_static_dates 'today:Today' \
                             'yesterday:Yesterday' \
                             'tomorrow:Tomorrow' \
                             'sow:Start of week' \
                             'soww:Start of work week' \
                             'socw:Start of calendar week' \
                             'som:Start of month' \
                             'soq:Start of quarter' \
                             'soy:Start of year' \
                             'eow:End of week' \
                             'eoww:End of work week' \
                             'eocw:End of calendar week' \
                             'eom:End of month' \
                             'eoq:End of quarter' \
                             'eoy:End of year' \
                             'mon:Monday' \
                             'tue:Tuesday'\
                             'wed:Wednesday' \
                             'thu:Thursday' \
                             'fri:Friday' \
                             'sat:Saturday' \
                             'sun:Sunday' \
                             'goodfriday:Good Friday' \
                             'easter:Easter' \
                             'eastermonday:Easter Monday' \
                             'ascension:Ascension' \
                             'pentecost:Pentecost' \
                             'midsommar:Midsommar' \
                             'midsommarafton:Midsommarafton' \
                             'later:Later' \
                             'someday:Some Day'
set __fish_task_static_dates (printf -- '%s\n' $__fish_task_static_dates | sed 's/:/\t/' | string collect)
set __fish_task_static_reldates 'hrs:n hours' \
                                'day:n days' \
                                # '1st:first' \
                                # '2nd:second' \
                                # '3rd:third' \
                                # 'th:4th, 5th, etc.' \
                                'wks:weeks'
set __fish_task_static_reldates (printf -- '%s\n' $__fish_task_static_reldates | sed 's/:/\t/' | string collect)
# the followings are actually not used for autocomplete, but to retrieve friendly description that aren't present in internal command
set  __fish_task_static_attr_desc_keys 'description' 'status' 'project' \
                                          'priority' 'due' 'recur' \
                                          'until' 'limit' 'wait' \
                                          'entry' 'end' 'start' \
                                          'scheduled' 'dependson'
set  __fish_task_static_attr_desc_vals 'Task description text' 'Status of task - pending, completed, deleted, waiting' \
                                          'Project name' 'Task priority' 'Due date' 'Recurrence frequency' 'Expiration date' \
                                          'Desired number of rows in report' 'Date until task becomes pending' \
                                          'Date task was created' 'Date task was completed/deleted' 'Date task was started' \
                                          'Date task is scheduled to start' 'Other tasks that this task depends upon'

# fish's auto-completion when multiple `complete` have supplied with '-k' flag, the last will be displayed first
__fish.task.complete config
__fish.task.complete attr_value
__fish.task.complete attr_name
__fish.task.complete tag
# __fish.task.complete command all
# __fish.task.complete command filter
# The following are static so we will expand it when initialised. Display underscore (internal) commands last
set -l __fish_task_static_commands_underscore (echo -e $__fish_task_static_commands_with_desc | grep '^[_]' | string collect | string escape)
set -l __fish_task_static_commands_normal (echo -e $__fish_task_static_commands_with_desc | grep '^[^_]' | string collect | string escape)
complete -c task -u -k -f -n "__fish.task.before_command" -a "$__fish_task_static_commands_underscore"
complete -c task -u -k -f -n "__fish.task.before_command" -a "$__fish_task_static_commands_normal"

if [ "$task_complete_task" = 'yes' ]
    __fish.task.complete task
end

if [ "$task_complete_id" = 'yes' ]
    __fish.task.complete id with_description
end
