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
