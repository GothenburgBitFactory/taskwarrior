# Taskwarrior

[![Travis build status](https://travis-ci.org/GothenburgBitFactory/taskwarrior.svg?branch=2.5.2)](https://travis-ci.org/GothenburgBitFactory/taskwarrior)

Thank you for taking a look at Taskwarrior!

Taskwarrior is a GTD, todo list, task management, command line utility with a
multitude of features. It is a portable, well supported and very active Open
Source project.

## Documentation

There is extensive online documentation.
You'll find all the details at [taskwarrior.org/docs](https://taskwarrior.org/docs)

At the site you'll find online documentation, downloads, news and more.

## Support

For support options, take a look at [taskwarrior.org/support](https://taskwarrior.org/support)

Please use pull requests, or alternately send your code patches to
[support@gothenburgbitfactory.org](mailto:support@gothenburgbitfactory.org)

## Branching Model

We use the following branching model:

* `master` is the stable branch. Building from here is the same as building
  from the latest tarball, or installing a binary package. No development is
  done on the `master` branch.

* `2.6.0` is the current development branch. All work is done here, and upon
  release it will be merged to `master`. This development branch is not stable,
  may not even build or pass tests, and should be treated accordingly.
  Make backups.

## Installing

There are many binary packages available, but to install from source requires:

* git
* cmake
* make
* C++ compiler, currently gcc 5.0+ or clang 3.4+ for full C++14 support

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

