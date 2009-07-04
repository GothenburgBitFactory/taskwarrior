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

_task_get_projects() {
    task _projects
}

_task_get_tags() {
    task _tags
}

_task() 
{
    local cur prev opts base

    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    opts="$(task _commands) $(task _ids)"

    case "${cur}" in
        pro*:*)
            local projects=$(_task_get_projects)
            local partial_project="${cur/*:/}"
            COMPREPLY=( $(compgen -W "${projects}" -- ${partial_project}) )
            return 0
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
    esac
      
    COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
    return 0
}
complete -F _task task
