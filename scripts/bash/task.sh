################################################################################
#
# Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#
################################################################################
#
# The routines will do completion of:
#
#    *) task subcommands
#    *) project names
#    *) tag names
#    *) aliases
#
# To use these routines:
#
#    1) Copy this file to somewhere (e.g. ~/.bash_completion.d/task.sh).
#    2) Add the following line to your .bashrc:
#        source ~/.bash_completion.d/task.sh
#
#    OR
#
#    3) Copy the file to /etc/bash_completion.d
#    4) source /etc/bash_completion
#
# To submit patches/bug reports:
#
#    *) Go to the project's website at
#
#       http://taskwarrior.org
#
################################################################################
#the following variable is substituted for by ../../test/bash_completion.t
taskcommand='task rc.verbose:nothing rc.confirmation:no rc.hooks:off'

_task_get_tags() {
    $taskcommand _tags
}

_task_get_config() {
    $taskcommand _config
}

_task_offer_priorities() {
    COMPREPLY=( $(compgen -W "L M H" -- ${cur/*:/}) )
}

_task_offer_projects() {
    COMPREPLY=( $(compgen -W "$($taskcommand _projects)" -- ${cur/*:/}) )
}

_task_offer_contexts() {
    COMPREPLY=( $(compgen -W "$($taskcommand _context) define delete list none show" -- $cur) )
}

_task_context_alias=$($taskcommand show | grep alias.*context | cut -d' ' -f1 | cut -d. -f2)

_task()
{
    local cur prev opts base

    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    if [ ${#COMP_WORDS[*]} -gt 2 ]
    then
        prev2="${COMP_WORDS[COMP_CWORD-2]}"
    else
        prev2=""
    fi
#   useful for debugging:
#   echo -e "\ncur='$cur'"
#   echo "prev='$prev'"
#   echo "prev2='$prev2'"

    abbrev_min=$($taskcommand show | grep "abbreviation.minimum" | awk {'print  $2'})
    commands_aliases=$(echo $($taskcommand _commands; $taskcommand _aliases) | tr " " "\n"|sort|tr "\n" " ")
    opts="$commands_aliases $($taskcommand _columns)"

    case "${prev}" in
        $_task_context_alias|cont|conte|contex|context)
                _task_offer_contexts
                return 0
                ;;
        :)
            case "${prev2}" in
                pri|prior|priori|priorit|priority)
                    if [ ${#prev2} -ge $abbrev_min ]; then
                        _task_offer_priorities
                    fi
                    return 0
                    ;;
                pro|proj|proje|projec|project)
                    if [ ${#prev2} -ge $abbrev_min ]; then
                        _task_offer_projects
                    fi
                    return 0
                    ;;
                rc)
                    # not activated when only "rc:" but is activated if anything after "rc:"
                   _filedir
                   return 0
                    ;;
                rc.data.location)
                   _filedir -d
                   return 0
                   ;;
            esac
            ;;
        *)
            case "${cur}" in
                pro:*|proj:*|proje:*|projec:*|project:*)
                    _task_offer_projects
                    return 0
                    ;;
                :)
                    case "${prev}" in
                        pri|prior|priori|priorit|priority)
                            if [ ${#prev} -ge $abbrev_min ]; then
                                _task_offer_priorities
                            fi
                            return 0
                            ;;
                        pro|proj|proje|projec|project)
                            if [ ${#prev} -ge $abbrev_min ]; then
                                _task_offer_projects
                            fi
                            return 0
                            ;;
                        rc)
                            # activated only when "rc:"
                            cur="" # otherwise ":" is passed.
                            _filedir
                            return 0
                            ;;
                        rc.data.location)
                            cur=""
                            _filedir -d
                            return 0
                            ;;
                    esac
                    ;;
                +*)
                    local tags=$(_task_get_tags | sed 's/^/+/')
                    COMPREPLY=( $(compgen -W "${tags}" -- ${cur}) )
                    return 0
                    ;;
                -*)
                    local tags=$(_task_get_tags | sed 's/^/-/')
                    COMPREPLY=( $(compgen -W "${tags}" -- ${cur}) )
                    return 0
                    ;;
                rc.*)
                    local config=$(_task_get_config | sed -e 's/^/rc\./' -e 's/$/:/')
                    COMPREPLY=( $(compgen -W "${config}" -- ${cur}) )
                    return 0
                    ;;
        *)
            case "${prev}" in
                import)
                    COMPREPLY=( $(compgen -o "default" -- ${cur}) )
                    return 0
                    ;;
            esac
            ;;
        esac
        ;;
    esac

    COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
    return 0
}
complete -o nospace -F _task task
