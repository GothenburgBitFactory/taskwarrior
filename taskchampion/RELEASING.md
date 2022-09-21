# Release process

1. Ensure the changelog is updated with everything from the `.changelogs` directory. `python3 ./scripts/changelog.py build` will output a Markdown snippet to include in `CHANGELOG.md` then `rm .changelog/*.txt`
1. Run `git pull upstream main`
1. Run `cargo test`
1. Run `cargo clean && cargo clippy`
1. Run `mdbook test docs`
1. Update `version` in `*/Cargo.toml`.  All versions should match.
1. Run `cargo build --release`
1. Commit the changes (Cargo.lock will change too) with comment `vX.Y.Z`.
1. Run `git tag vX.Y.Z`
1. Run `git push upstream`
1. Run `git push --tags upstream`
1. Run `( ./build-docs.sh )`
1. Run `(cd taskchampion; cargo publish)` (note that the other crates do not get published)
1. Navigate to the tag in the GitHub releases UI and create a release with general comments about the changes in the release
1. Upload `./target/release/task` and `./target/release/task-sync-server` to the release
