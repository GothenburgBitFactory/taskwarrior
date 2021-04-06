# Taskwarrior

[![GitHub Actions build status](https://github.com/GothenburgBitFactory/taskwarrior/workflows/tests/badge.svg?branch=2.6.0)](https://github.com/GothenburgBitFactory/taskwarrior/actions)

Thank you for taking a look at Taskwarrior!

Taskwarrior is a command line task list management utility with a [multitude of
features](https://taskwarrior.org/docs/), developed as a portable open source project
with an active and quite vast [ecosystem of tools, hooks and
extensions](https://taskwarrior.org/tools/).

Taskwarrior runs on a wide range of [Linux/Unix systems, Mac OS and
Windows](https://taskwarrior.org/download/).

## Documentation

The [online documentation](https://taskwarrior.org/docs), downloads, news and
more are available on our website, [taskwarrior.org](https://taskwarrior.org).

We also recommend following [@taskwarrior on
Twitter](https://twitter.com/taskwarrior), where we share info about new
features, releases and various tips and tricks for new Taskwarrior users.

## Support

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

In case of errors with libshared - URL pointing to git.tasktools.org in either .git/config or .gitmodules:

    $ sed -i 's/git.tasktools.org\/TM/github.com\/GothenburgBitFactory/' .git/config
    $ git submodule update

or

    $ sed -i 's/git.tasktools.org\/TM/github.com\/GothenburgBitFactory/' .gitmodules
    $ git submodule init
    $ git submodule update

Then build:

    $ cmake -DCMAKE_BUILD_TYPE=release .
    ...
    $ make
    ...
    [$ make test]
    ...
    $ sudo make install

## Contributing

Your contributions are especially welcome.
Whether it comes in the form of code patches, ideas, discussion, bug reports, encouragement or criticism, your input is needed.

Visit [Github](https://github.com/GothenburgBitFactory/taskwarrior) and participate in the future of Taskwarrior.

## License

Taskwarrior is released under the MIT license.
For details check the [LICENSE](LICENSE) file.
