# tab completions for the Fish shell <http://fishshell.org>
#
# taskwarrior - a command line task list manager.
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
#  Copyright 2009 - 2012 Mick Koch <kchmck@gmail.com>
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
