# Changelog

## [Unreleased]

Note: unreleased change log entries are kept in `.changelogs/` directory in repo root, and can be added with `./script/changelog.py add "Added thing for reason"

## 0.3.0 - 2021-01-11
- Flexible named reports
- Updates to the TaskChampion crate API
- Usability improvements

## 0.2.0 - 2020-11-30

This release is the first "MVP" version of this tool. It can do basic task operations, and supports a synchronization. Major missing features are captured in issues, but briefly:

    better command-line API, similar to TaskWarrior
    authentication of the replica / server protocol
    encryption of replica data before transmission to the server
    lots of task features (tags, annotations, dependencies, ..)
    lots of CLI features (filtering, modifying, ..)
