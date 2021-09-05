# Welcome

This application is still in a pre-release state.
That means it's very open to contributions, and we'd love to have your help!

It also means that things are changing quickly, and lots of stuff is planned that's not quite done yet.
If you would like to work on TaskChampion, please contact the developers (via the issue tracker) before spending a lot of time working on a pull request.
Doing so may save you some wasted time and frustration!

A good starting point might be one of the issues tagged with ["good first issue"][first].

[first]: https://github.com/taskchampion/taskchampion/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22

# Other Ways To Help

The best way to help this project to grow is to help spread awareness of it.
Tell your friends, post to social media, blog about it -- whatever works best!

Other ideas;
 * Improve the documentation where it's unclear or lacking some information
 * Build and maintain tools that integrate with TaskChampion

# Development Guide

TaskChampion is a typical Rust application.
To work on TaskChampion, you'll need to [install the latest version of Rust](https://www.rust-lang.org/tools/install).
Once you've done that, run `cargo build` at the top level of this repository to build the binaries.
This will build `task` and `taskchampion-sync-server` executables in the `./target/debug` directory.
You can build optimized versions of these binaries with `cargo build --release`, but the performance difference in the resulting binaries is not noticeable, and the build process will take a long time, so this is not recommended.

## Running Test

It's always a good idea to make sure tests run before you start hacking on a project.
Run `cargo test` from the top-level of this repository to run the tests.

## Read the Source

Aside from that, start reading the docs and the source to learn more!
The book documentation explains lots of the concepts in the design of TaskChampion.
It is linked from the README.

There are three crates in this repository.
You may be able to limit the scope of what you need to understand to just one crate.
 * `taskchampion` is the core functionality of the application, implemented as a library
 * `taskchampion-cli` implements the command-line interface (in `cli/`)
 * `taskchampion-sync-server` implements the synchronization server (in `sync-server/`)
 
 You can generate the documentation for the `taskchampion` crate with `cargo doc --release --open -p taskchampion`.
 
## Making a Pull Request
 
We expect contributors to follow the [GitHub Flow](https://guides.github.com/introduction/flow/).
Aside from that, we have no particular requirements on pull requests.
Make your patch, double-check that it's complete (tests? docs? documentation comments?), and make a new pull request.

Any non-trivial change (particularly those that change the behaviour of the application, or change the API) should be noted in the projects changelog.
In order to manage this, changelog entries are stored as text files in the `.changelog/` directory at the repository root.

To add a new changelog entry, you can simply run `python3 ./script/changelog.py add "Fixed thingo to increase zorbloxification [Issue #2](http://example.com)`

This creates a file named `./changelogs/yyyy-mm-dd-branchname.md` (timestamp, current git branch) which contains a markdown snippet.

If you don't have a Python 3 intepreter installed, you can simply create this file manually. It should contain a list item like `- Fixed thingo [...]`

Periodically (probably just before release), these changelog entries are concatenated combined together and added into the `CHANGELOG.md` file.
