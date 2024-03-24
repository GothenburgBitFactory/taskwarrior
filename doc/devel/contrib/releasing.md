# Releasing Taskwarrior

To release Taskwarrior, follow this process:

- Create a release PR
    - Update version in CMakeLists.txt
    - Update Changelog
    - get this merged
- On `develop` after that PR merges, create a release tarball:
  - `git clone . release-tarball`
  - `cd release-tarball/`
  - `cmake -S. -Bbuild`
  - `make -Cbuild package_source`
  - copy build/task-*.tar.gz elsewhere and delete the `release-tarball` dir
  - NOTE: older releases had a `test-*.tar.gz` but it's unclear how to generate this
- Update `stable` to the released commit and push upstream
- Tag the commit as vX.Y.Z and push the tag upstream
- Find the tag under https://github.com/GothenburgBitFactory/taskwarrior/tags and create a release from it
  - Give it a clever title if you can think of one; refer to previous releases
  - Include the tarball from earlier
- Add a new item in `content/news` on https://github.com/GothenburgBitFactory/tw.org
