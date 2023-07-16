# Coding Style

The coding style used for the Taskwarrior, Taskserver, and other codebases is deliberately kept simple and a little vague.
This is because there are many languages involved (C++, C, Python, sh, bash, HTML, troff and more), and specіfying those would be a major effort that detracts from the main focus which is improving the software.

Instead, the general guideline is simply this:

Make all changes and additions such that they blend in perfectly with the surrounding code, so it looks like only one person worked on the source, and that person is rigidly consistent.

To be a little more explicit:

## C++

-   All functionality in C++17 is allowed.

-   Indent C++ code using two spaces, no tabs

-   Surround operators and expression terms with a space.
    This includes a space between a function name and its list of arguments.

-   No cuddled braces

-   Class names are capitalized, variable names are not

## Python

Follow [PEP8](https://www.python.org/dev/peps/pep-0008/) as much as possible.

## Rust

Rust code should be formatted with `rustfmt` and generally follow Rust style guidelines.
