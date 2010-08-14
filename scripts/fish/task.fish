# Task <http://taskwarrior.org> tab completions for the Fish shell
# <http://fishshell.org>.
#
# Copy this script to ~/.config/fish/completions/task.fish, open a new shell,
# and enjoy.
#
# Objects completed:
#  * Commands
#  * Projects
#  * Priorities
#  * Tags
#  * Attribute names and modifiers
#
# License:
#  Copyright 2009 Mick Koch <kchmck@gmail.com>
#
#  This script is free software. It comes without any warranty, to the extent
#  permitted by applicable law. You can redistribute it and/or modify it under
#  the terms of the Do What The Fuck You Want To Public License, Version 2, as
#  published by Sam Hocevar. See http://sam.zoy.org/wtfpl/COPYING for more
#  details.

function __fish.task.bare
    test (count (commandline -c -o)) -eq 1
end

function __fish.task.complete
    complete -c task -u $argv
end

function __fish.task.head
    task _ids
    task _commands
end

function __fish.task.attrs
    echo project
    echo priority
    echo due
    echo recur
    echo until
    echo limit
    echo wait
    echo rc
end

function __fish.task.mods
    echo before
    echo after
    echo over
    echo under
    echo none
    echo is
    echo isnt
    echo has
    echo hasnt
    echo startswith
    echo endswith
    echo word
    echo noword
end

function __fish.task.combos
    echo $argv[1]:$argv[2]

    for mod in (__fish.task.mods)
        echo $argv[1].$mod:$argv[2]
    end
end

function __fish.task.combos.simple
    __fish.task.combos $argv ""
end

function __fish.task.projects
    __fish.task.combos.simple project

    for project in (task _projects)
        __fish.task.combos project $project
    end
end

function __fish.task.priorities
    __fish.task.combos.simple priority

    for priority in H M L
        __fish.task.combos priority $priority
    end
end

function __fish.task.rc
    echo rc:

    for value in (task _config)
        echo rc.$value:
    end
end

function __fish.task.tags
    for tag in (task _tags)
        echo +$tag
        echo -$tag
    end
end

function __fish.task.match
    __fish.task.attrs | grep \^(echo $argv | sed -E "s/(\w+).+/\1/")
end

function __fish.task.attr
    for attr in (__fish.task.match $argv)
        switch $attr
            case project
                __fish.task.projects
            case priority
                __fish.task.priorities
            case rc
                __fish.task.rc
            case "*"
                __fish.task.combos.simple $attr
        end
    end
end

function __fish.task.body
    set token (commandline -ct)

    if test -n $token
        __fish.task.attr $token
    else
        __fish.task.attrs
    end

    __fish.task.tags
end

__fish.task.complete -f -n __fish.task.bare -a "(__fish.task.head)"
__fish.task.complete -f -n "not __fish.task.bare" -a "(__fish.task.body)"
