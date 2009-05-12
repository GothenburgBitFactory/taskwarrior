_task() 
{
    local cur prev opts base

    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    opts="add append annotate completed delete undelete info start stop done undo projects tags summary history ghistory next calendar stats import export color version help list long ls newest oldest overdue active"

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
