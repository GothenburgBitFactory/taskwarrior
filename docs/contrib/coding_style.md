---
title: Coding Style
---

The coding style used for the Taskwarrior, Taskserver, and other codebases is
deliberately kept simple and a little vague. This is because there are many
languages involved (C++, C, Python, sh, bash, HTML, troff and more), and
specіfying those would be a major effort that detracts from the main focus which
is improving the software.

Instead, the general guideline is simply this:

Make all changes and additions such that they blend in perfectly with the
surrounding code, so it looks like only one person worked on the source, and
that person is rigidly consistent.

To be a little more explicit, the common elements across the languages are:

-   Indent code using two spaces, no tabs
-   With Python, follow [PEP8](https://www.python.org/dev/peps/pep-0008/) as
    much as possible
-   Surround operators and expression terms with a space
-   No cuddled braces
-   Class names are capitalized, variable names are not

We target Python 2.7 so that our test suite runs on the broadest set of
platforms. This will likely change in the future and 2.7 will be dropped.

We can safely target C++11 because all the default compilers on our supported
platforms are ready. Feel free to use C++14 and C++17 provided that all build
platforms support this.
