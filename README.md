<div align="center">
<img src="https://avatars.githubusercontent.com/u/36100920?s=200&u=24da05914c20c4ccfe8485310f7b83049407fa9a&v=4"></br>

[![GitHub Actions build status](https://github.com/GothenburgBitFactory/taskwarrior/workflows/tests/badge.svg?branch=develop)](https://github.com/GothenburgBitFactory/taskwarrior/actions)
[![Release](https://img.shields.io/github/v/release/GothenburgBitFactory/taskwarrior)](https://github.com/GothenburgBitFactory/taskwarrior/releases/latest)
[![Release date](https://img.shields.io/github/release-date/GothenburgBitFactory/taskwarrior)](https://github.com/GothenburgBitFactory/taskwarrior/releases/latest)
[![GitHub Sponsors](https://img.shields.io/github/sponsors/GothenburgBitFactory?color=green)](https://github.com/sponsors/GothenburgBitFactory/)
</br>
[![Twitter](https://img.shields.io/twitter/follow/taskwarrior?style=social)](https://twitter.com/taskwarrior)
</div>

## Taskwarrior
![OS-X downloads](https://img.shields.io/homebrew/installs/dy/task?label=OS-X%20downloads)
![Github downloads](https://img.shields.io/github/downloads/GothenburgBitFactory/taskwarrior/total?label=Github%20downloads)
![Linux downloads](https://img.shields.io/badge/Linux%20downloads-unknown-gray)

Taskwarrior is a command line task list management utility with a [multitude of
features](https://taskwarrior.org/docs/), developed as a portable open source project
with an active and quite vast [ecosystem of tools, hooks and
extensions](https://taskwarrior.org/tools/).

## Install
[![Arch](https://img.shields.io/archlinux/v/community/x86_64/task)](https://archlinux.org/packages/community/x86_64/task/)
[![Debian](https://img.shields.io/debian/v/task/testing)](https://packages.debian.org/search?keywords=task&searchon=names&suite=all&section=all)
[![Fedora](https://img.shields.io/fedora/v/task)](https://bodhi.fedoraproject.org/updates/?packages=task)
[![Homebrew](https://img.shields.io/homebrew/v/task)](https://formulae.brew.sh/formula/task#default)
[![Ubuntu](https://img.shields.io/ubuntu/v/task)](https://packages.ubuntu.com/search?keywords=task&searchon=names&suite=hirsute&section=all)

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
[![Discord](https://img.shields.io/discord/796949983734661191?label=discord)](https://discord.gg/eRXEHk8w62)
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

* `stable` is a branch containing the content of the latest release. Building
  from here is the same as building from the latest tarball, or installing a
  binary package. No development is done on the `stable` branch.

* `develop` is the current development branch. All work is done here, and upon
  release it will be merged to `stable`. While development branch is not
  stable, we utilize CI to ensure we're at least not merging improvements that
  break existing tests, and hence should be relatively safe. We still recommend
  making backups when using the development branch.

## Installing

There are many binary packages available, but to install from source requires:

* git
* cmake
* make
* C++ compiler, currently gcc 7.1+ or clang 5.0+ for full C++17 support
* libuuid
* GnuTLS (optional, required for sync)

Download the tarball, and expand it:

    $ curl -O https://taskwarrior.org/download/task-2.6.2.tar.gz
    $ tar xzf task-2.6.2.tar.gz
    $ cd task-2.6.2

Or clone this repository:

    $ git clone --recursive -b stable https://github.com/GothenburgBitFactory/taskwarrior.git
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
[![Contributors](https://img.shields.io/github/contributors/GothenburgBitFactory/taskwarrior)](https://github.com/GothenburgBitFactory/taskwarrior/graphs/contributors)
[![Milestone progress](https://img.shields.io/github/milestones/progress/GothenburgBitFactory/taskwarrior/26?label=current%20milestone%20issues)](https://github.com/GothenburgBitFactory/taskwarrior/milestone/26)
[![Good first issus](https://img.shields.io/github/issues/GothenburgBitFactory/taskwarrior/good%20first%20issue)](https://github.com/GothenburgBitFactory/taskwarrior/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22)

Your contributions are especially welcome.
Whether it comes in the form of code patches, ideas, discussion, bug reports, encouragement or criticism, your input is needed.

Visit [Github](https://github.com/GothenburgBitFactory/taskwarrior) and participate in the future of Taskwarrior.

## Sponsoring
[![GitHub Sponsors](https://img.shields.io/github/sponsors/GothenburgBitFactory?color=green)](https://github.com/sponsors/GothenburgBitFactory/)

Taskwarrior is a result of work of mostly small group of volunteers, and has been in development since 2006.

If you are a happy Taskwarrior user, please consider [sponsoring us through
Github Sponsors](https://github.com/sponsors/GothenburgBitFactory/).

Every sponsorship matters, as it directly increases the number of hours core
developers can contribute to the project and makes the project more sustainable.

## License

Taskwarrior is released under the MIT license.
For details check the [LICENSE](LICENSE) file.
