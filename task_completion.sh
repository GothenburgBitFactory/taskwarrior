#
# bash completion support for task 1.7.0-2
# Copyright (C) 2009 Federico Hernandez <ultrafredde@gmail.com>
# Distributed under the GNU General Public License, version 2.0
#
# The routines will do completion of:
#
#    *) task subcommands
#    *) local and remote tag names
#
# To use these routines:
#
#    1) Copy this file to somewhere (e.g. ~/.task-completion.sh).
#    2) Added the following line to your .bashrc:
#        source ~/.task-completion.sh
#
#    OR
#
#    3) Copy the file to /etc/bash_complettion.d
#    4) source /etc/bash_completion
#
# To submit patches/bug reports:
#
#    *) Send them to the mailing list:
#
#       taskprogram@googlegroups.com
#       
#    *) CC the all patchesi/bug reports to:
#
#       Federico Hernandez <ultrafredde@gmail.com>
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
