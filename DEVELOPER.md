# How to Build Taskwarrior

## Satisfy the Requirements:
 * CMake 3.0 or later
 * gcc 7.0 or later, clang 6.0 or later, or a compiler with full C++17 support
 * libuuid (if not on macOS)
 * gnutls (optional)
 * python 3 (optional, for running the test suite)

## Obtain and build code:
```
    $ git clone --recursive https://github.com/GothenburgBitFactory/taskwarrior taskwarrior.git
    $ cd taskwarrior.git
    $ git checkout develop               # Latest dev branch
    $ git submodule init                 # This is now done by cmake as a test
    $ git submodule update               # Update the libhsared.git submodule
    $ cmake -DCMAKE_BUILD_TYPE=debug .   # debug or release. Default: neither
    $ make VERBOSE=1 -j4                 # Shows details, builds using 4 jobs
                                         # Alternately 'export MAKEFLAGS=-j 4'
```
## Running Test Suite:
```
    $ cd test
    $ make VERBOSE=1                     # Shows details
    $ ./run_all                          # Runs all tests silently > all.log
    $ ./problems                         # Enumerate test failures in all.log
```

  Note that any development should be performed using a git clone, and the
  current development branch. The source tarballs do not reflect HEAD, and do
  not contain the test suite.

  If you send a patch (support@gothenburgbitfactory.org), make sure that patch is made
  against git HEAD on the development branch. We cannot apply patches made
  against the tarball source, or master.


# General Statement
  This file is intended to convey the current efforts, priorities and needs of
  the code base. It is for anyone looking for a way to start contributing.
  Here are many ways to contribute that may not be obvious:

  * Use Taskwarrior, become familiar with it, and make suggestions. There are
    always ongoing discussions about new features and changes to existing
    features.

  * Join us in the #taskwarrior IRC channel on freenode.net or libera.chat.
    Many great ideas, suggestions, testing and discussions have taken place
    there. It is also the quickest way to get help, or confirm a bug.

  * Review documentation: there are man pages, online articles, tutorials and
    so on, and these may contain errors, or they may not convey ideas in the
    best way. Perhaps you can help improve it. Contact us - documentation is
    a separate effort from the code base, and includes all web sites, and all
    are available as git repositories.

  * Take a look at the bug database, and help triage the bug list. This is a
    review process that involves confirming bugs, providing additional data,
    information or analysis. Bug triage is very useful and much needed. You
    could check to see that an old bug is still relevant - sometimes they are
    not.

  * Review the source code, and point out inefficiencies, problems, unreadable
    functions, bugs and assumptions.

  * Fix a bug. For this you'll need C++ and Git skills. We welcome all bug
    fixes, provided the work is done well and doesn't create other problems or
    introduce new dependencies. We recommend talking to us before starting.
    Seriously.

  * Add unit tests. Unit tests are possibly the most useful contributions of
    all, because they not only improve the quality of the code, but prevent
    future regressions, therefore maintaining quality of subsequent releases.
    Plus, broken tests are a great motivator for us to fix the causal defect.
    You'll need Python skills.

  * Add a feature. Well, let's be very clear about this: adding a feature is
    not usually well-received, and if you add a feature and send a patch, it
    will most likely be rejected. The reason for this is that there are many
    efforts under way, in various code branches. There is a very good chance
    that the feature you add is either already in progress, or being done in a
    way that is more fitting when considering other work in progress. So if
    you want to add a feature, please don't. Start by talking to us, and find
    out what is currently under way or planned. You might find that we've
    already rejected such a feature for some very good reasons. So please
    check first, so we don't duplicate effort or waste anyone's time.

  * Spread the word. Help others become more effective at managing tasks.

  * Encouragement. Tell us what works for you, and what doesn't. Tell us about
    your methodology for managing tasks. It's all useful information.

  * Request a feature. This not only tells us that you think something is
    missing from the software, but gives us insights into how you use it.
    Plus, you might get your feature implemented.

# Unit Tests Needed
  There are always more unit tests needed. More specifically, better unit tests
  are always needed. The convention is that there are four types of unit test:

  1. High level tests that exercise large features, or combinations of commands.
     For example, dependencies.t runs through a long list of commands that test
     dependencies, but do so by using 'add', 'modify', 'done' and 'delete'.
  1. Regression tests that ensure certain bugs are fixed and stay fixed. These
     tests are named tw-NNNN.t where NNNN refers to the bug number. While it is
     not worth creating tests for small fixes like typos, it is for logic
     changes.
  1. Small feature tests. When small features are added, we would like small,
     low-level feature tests named feature.t, with a descriptive name and
     focused tests.
  1. Code tests. These are tests written in C++ that exercise C++ objects, or
     function calls. These are the lowest level tests. It is important that
     these kind of tests be extensive and thorough, because the software depends
     on this code the most.

  The tests are written in Python, Bash and C++, and all use TAP.

## Tests needed

  * Take a look at the bug database (https://github.com/GothenburgBitFactory/taskwarrior/issues)
    and notice that many issues, open and closed, have the "needsTest" label.
    These are things that we would like to see in the test suite, as regression
    tests.

  All new unit tests should follow the test/template.t standard.

# Patches
  Patches are encouraged and welcomed. Either send a pull request on Github or
  email a patch to support@taskwarrior.org. A good patch:

    * Maintains the MIT license, and does not contain code lifted from other
      sources. You will have written 100% of the code in the patch, otherwise
      we cannot maintain the license.
    * Precisely addresses one issue only.
    * Doesn't break unit tests. This means yes, run the unit tests.
    * Doesn't introduce dependencies.
    * Is accompanied by new or updated unit tests, where appropriate.
    * Is accompanied by documentation changes, where appropriate.
    * Conforms to the prevailing coding standards - in other words, it should
      fit in with the existing code.

  A patch may be rejected for violating any of the above rules, and more.
  Bad patches may be accepted and modified depending on work load and mood. It
  is possible that a patch may be rejected because it conflicts in some way with
  plans or upcoming changes. Check with us first, before sinking time and effort
  into a patch.
