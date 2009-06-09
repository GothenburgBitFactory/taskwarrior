# bash completion support for task
#
# Copyright 2009 Federico Hernandez
# All rights reserved.
#
# This script is part of the task project.
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

_task() 
{
    local cur prev opts base

    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    opts="active add annotate append calendar color completed delete done duplicate edit export ghistory help history import info list long ls newest next oldest overdue projects start stats stop summary tags timesheet undelete undo version"

    case "${prev}" in
      ls|list|long)
        if [[ ${cur} == +* ]] ; then
          local tags=$( task tags | egrep -v 'tags|^$'|sed 's/^/+/' )
          COMPREPLY=( $(compgen -W "${tags}" -- ${cur}) )
          return 0
        fi
        ;;
    esac
      
    COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
    return 0
}
complete -F _task task
