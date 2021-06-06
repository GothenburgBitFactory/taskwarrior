<div align="center">
<img src="https://avatars.githubusercontent.com/u/36100920?s=200&u=24da05914c20c4ccfe8485310f7b83049407fa9a&v=4"></br>

[![GitHub Actions build status](https://github.com/GothenburgBitFactory/taskwarrior/workflows/tests/badge.svg?branch=2.6.0)](https://github.com/GothenburgBitFactory/taskwarrior/actions)
[![Release](https://img.shields.io/github/v/release/GothenburgBitFactory/taskwarrior)](https://github.com/GothenburgBitFactory/taskwarrior/releases/latest)
[![Release date](https://img.shields.io/github/release-date/GothenburgBitFactory/taskwarrior)](https://github.com/GothenburgBitFactory/taskwarrior/releases/latest)
![Commits since release](https://img.shields.io/github/commits-since/GothenburgBitFactory/taskwarrior/latest)
</br>
![OS-X downloads](https://img.shields.io/homebrew/installs/dy/task?label=OS-X%20downloads)
![Github downloads](https://img.shields.io/github/downloads/GothenburgBitFactory/taskwarrior/total?label=Github%20downloads)
![Linux downloads](https://img.shields.io/badge/Linux%20downloads-unknown-gray)
</br>
[![Twitter](https://img.shields.io/twitter/follow/taskwarrior?style=social)](https://twitter.com/taskwarrior)
</div>

## Taskwarrior

Taskwarrior is a command line task list management utility with a [multitude of
features](https://taskwarrior.org/docs/), developed as a portable open source project
with an active and quite vast [ecosystem of tools, hooks and
extensions](https://taskwarrior.org/tools/).

## Install
![Arch](https://img.shields.io/archlinux/v/community/x86_64/task)
![Debian](https://img.shields.io/debian/v/task/testing)
![Fedora](https://img.shields.io/fedora/v/task)
![Homebrew](https://img.shields.io/homebrew/v/task)
![Ubuntu](https://img.shields.io/ubuntu/v/task)

Taskwarrior is packaged on a wide range of [Linux/Unix systems, Mac OS and
Windows](https://taskwarrior.org/download/). Check out the latest available
packages in repositories of your OS distribution of choice [on
Repology](https://repology.org/project/taskwarrior/versions).

Alternatively, you can build Taskwarrior from source.

## Documentation

The [online documentation](https://taskwarrior.org/docs), downloads, news and
more are available on our website, [taskwarrior.org](https://taskwarrior.org).

We also recommend following [@taskwarrior on
Twitter](https://twitter.com/taskwarrior), where we share info about new
features, releases and various tips and tricks for new Taskwarrior users.

## Community
[![Twitter](https://img.shields.io/twitter/follow/taskwarrior?style=social)](https://twitter.com/taskwarrior)
[![Reddit](https://img.shields.io/reddit/subreddit-subscribers/taskwarrior?style=social)](https://reddit.com/r/taskwarrior/)
[![Libera.chat](https://img.shields.io/badge/IRC%20libera.chat-online-green)](https://web.libera.chat/#taskwarrior)
![Discord](https://img.shields.io/discord/796949983734661191?label=discord)
[![Github discussions](https://img.shields.io/github/discussions/GothenburgBitFactory/taskwarrior?label=GitHub%20discussions)](https://github.com/GothenburgBitFactory/taskwarrior/discussions)

Taskwarrior has a lively community on many places on the internet.

Best place to ask questions is our [discussions forum on
Github](https://github.com/GothenburgBitFactory/taskwarrior/discussions). For
other support options, take a look at
[taskwarrior.org/support](https://taskwarrior.org/support)

For code contributions, please use pull requests, or alternately send your code patches to
[support@gothenburgbitfactory.org](mailto:support@gothenburgbitfactory.org)

## Branching Model

We use the following branching model:

* `master` is the stable branch. Building from here is the same as building
  from the latest tarball, or installing a binary package. No development is
  done on the `master` branch.

* `2.6.0` is the current development branch. All work is done here, and upon
  release it will be merged to `master`. This development branch is not stable,
  and should be treated accordingly. Make backups.

## Installing

There are many binary packages available, but to install from source requires:

* git
* cmake
* make
* C++ compiler, currently gcc 7.1+ or clang 5.0+ for full C++17 support

Download the tarball, and expand it:

    $ curl -O https://taskwarrior.org/download/task-2.6.0.tar.gz
    $ tar xzf task-2.6.0.tar.gz
    $ cd task-2.6.0

Or clone this repository:

    $ git clone --recursive -b 2.6.0 https://github.com/GothenburgBitFactory/taskwarrior.git
    $ cd taskwarrior

Then build:

    $ cmake -DCMAKE_BUILD_TYPE=release .
    ...
    $ make
    ...
    [$ make test]
    ...
    $ sudo make install

## Contributing
![Contributors](https://img.shields.io/github/contributors/GothenburgBitFactory/taskwarrior)
![Downloads](https://img.shields.io/github/downloads/GothenburgBitFactory/taskwarrior/total)
![OS-X downloads](https://img.shields.io/homebrew/installs/dy/task?label=OS-X%20downloads)

Your contributions are especially welcome.
Whether it comes in the form of code patches, ideas, discussion, bug reports, encouragement or criticism, your input is needed.

Visit [Github](https://github.com/GothenburgBitFactory/taskwarrior) and participate in the future of Taskwarrior.

## License

Taskwarrior is released under the MIT license.
For details check the [LICENSE](LICENSE) file.
