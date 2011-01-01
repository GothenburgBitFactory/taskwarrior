# bash completion support for taskwarrior
#
# Copyright 2009 - 2011 Federico Hernandez
# All rights reserved.
#
# This script is part of the taskwarrior project.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the
#
#     Free Software Foundation, Inc.,
#     51 Franklin Street, Fifth Floor,
#     Boston, MA
#     02110-1301
#     USA
#
# The routines will do completion of:
#
#    *) task subcommands
#    *) project names
#    *) tag names
#
# To use these routines:
#
#    1) Copy this file to somewhere (e.g. ~/.bash_completion.d/.task_completion.sh).
#    2) Added the following line to your .bashrc:
#        source ~/.bash_completion.d/task_completion.sh
#
#    OR
#
#    3) Copy the file to /etc/bash_complettion.d
#    4) source /etc/bash_completion
#
# To submit patches/bug reports:
#
#    *) Go to the projects website at
#
#       http://taskwarrior.org
#

_task_get_tags() {
    task _tags
}

_task_get_config() {
    task _config
}

_task_offer_projects() {
    COMPREPLY=( $(compgen -W "$(task _projects)" -- ${cur/*:/}) )
}

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
#   echo -e "\ncur='$cur'"
#   echo "prev='$prev'"
#   echo "prev2='$prev2'"

    opts="$(task _commands) $(task _ids)"

    case "${prev}" in
        :)
            case "${prev2}" in
                pro*)
                    _task_offer_projects
                    return 0
                    ;;
            esac
            ;;
        *)
            case "${cur}" in
                pro*:*)
                    _task_offer_projects
                    return 0
                    ;;
                :)
                    case "${prev}" in
                        pro*)
                            _task_offer_projects
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
							  merge)
								 local servers=$(_task_get_config | grep merge | grep uri | sed 's/^merge\.\(.*\)\.uri/\1/')
								 COMPREPLY=( $(compgen -W "${servers}" -- ${cur}) )
								 _known_hosts_real -a "$cur"
						       return 0
						       ;;
							  push)
								 local servers=$(_task_get_config | grep push | grep uri | sed 's/^push\.\(.*\)\.uri/\1/')
								 COMPREPLY=( $(compgen -W "${servers}" -- ${cur}) )
								 _known_hosts_real -a "$cur"
						       return 0
						       ;;
							  pull)
								 local servers=$(_task_get_config | grep pull | grep uri | sed 's/^pull\.\(.*\)\.uri/\1/')
								 COMPREPLY=( $(compgen -W "${servers}" -- ${cur}) )
								 _known_hosts_real -a "$cur"
						       return 0
						       ;;
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
