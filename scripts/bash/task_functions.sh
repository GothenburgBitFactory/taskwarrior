CONFIRM_PROMPT='are you sure'
task_config ()
{
    var="${1}"
    shift
    value="${@}"
    echo y | task config $var "$value" | grep -iv "$CONFIRM_PROMPT"
}
task_vars ()
{
    task _show | grep "^.*${1}.*=" | cut -d'=' -f1
}
task_get ()
{
    task _show | grep "^${1}=" | cut -d'=' -f2
}
task_color ()
{
    color="${1}"
    shift
    text="${@}"
    task rc.verbose=nothing rc._forcecolor=yes color ${color} | grep 'task color' | \
       tail -n 1 | sed -e 's/^  //' -e "s/task color ${color}/${text}/"
}


