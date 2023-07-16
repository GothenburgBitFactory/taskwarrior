Software development typically requires a standardized branching model, to manage complexity and parallel efforts.
The branching model can be a source of confusion for developers, so this document describes how branching is used.

We use the following branching model:

* `develop` is the current development branch. All work is done here, and upon
  release it will be branched to a release branch. While `develop` is not
  stable, we utilize CI to ensure we're at least not merging improvements that
  break existing tests, and hence should be relatively safe. We still recommend
  making backups when using the development branch.

* The most recent minor release is in a branch named after the release, e.g., `2.7.0`.
  This branch is used for bug-fixes of the latest release.
