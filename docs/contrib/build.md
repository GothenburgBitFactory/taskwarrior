---
title: How To Build Taskwarrior
---

This is for developers.
Specifically those who know how to use tools, satisfy dependencies, and want to set up a development environment.
It is not user-friendly.

You\'ll need these tools:

-   [git](https://git-scm.com/)
-   [cmake](https://cmake.org)
-   make
-   C++ compiler, currently gcc 4.7+ or clang 3.3+ for full C++11 support
-   Python 2.7 or later (for tests)
-   Bash (for tests)

You\'ll need these libraries:

-   [GnuTLS](https://www.gnutls.org/)
-   libuuid (unless on Darwin/BSD)

Specifically the development versions, `uuid-dev` on Debian, for example.


# Cloning the Repo

    $ git clone https://github.com/GothenburgBitFactory/taskwarrior.git taskwarrior.git


# Building the Stable Version

The master always represents the more recently released version, and should be
your preferred choice.

    $ cd taskwarrior.git
    $ git checkout master                # Master is the stable branch.
    $ cmake -DCMAKE_BUILD_TYPE=release . # 'release' for performance.
    $ make                               # Just build it.


# Building the Dev Branch

The dev branch is always the highest numbered branch, in this example, `2.6.0`.

    $ cd taskwarrior.git
    $ git checkout 2.6.0                 # Dev branch
    $ git submodule init                 # Register submodule
    $ git submodule update               # Get the submodule
    $ cmake -DCMAKE_BUILD_TYPE=debug .   # debug or release, default: neither
    $ make VERBOSE=1                     # Shows details

Build the debug type if you want symbols in the binary.


# Running the Test Suite

There are scripts to facilitate running the test suite.
In particular, the [vramsteg](https://gothenburgbitfactory.org/projects/vramsteg) utility provides blinkenlights for test progress.

    $ cd taskwarrior.git/test
    $ make VERBOSE=1     # Shows details
    $ ./run_all          # Runs all tests silently > all.log
    $ ./problems         # Find errors in all.log


# Submitting a Patch

Talk to us first - make sure you are working on something that is wanted.
Patches will not be applied simply because you did the work.
Remember the various forms of documentation involved, and the test suite.
Work on the dev branch, not `master`.
When you are are ready to submit, do this:

    $ git commit

Follow the standard form for commit messages, which looks like this:

    Category: Short message

    - Details
    - Details

Here is a good example:

    TW-1636: UUID with numeric-only first segment is not parsed properly

    - Switched Nibbler::getUUID to Nibbler::getPartialUUID, which caused partial
      UUID matching to fail sometimes.
    - Changed precedence to search for UUID before ID, which solves the numeric
      UUID problem.

Create the patch using this:

    $ git format-patch HEAD^

Mail the patch to <taskwarrior-dev@googlegroups.com> or attach it to the appropriate ticket in the [bug tracker](https://github.com/GothenburgBitFactory/taskwarrior/issues).
If you do the latter, make sure someone knows about it, or it could go unnoticed.

Expect feedback.
It is unlikely your patch will be accepted unmodified.
Usually this is because you violated the coding style, worked in the wrong branch, or *forgot* about documentation and unit tests.
