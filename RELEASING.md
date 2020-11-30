# Release process

1. Run `cargo test`
1. Run `cargo clean && cargo clippy`
1. Run `mdbook test docs`
1. Update `version` in `*/Cargo.toml`.  All versions should match.  Commit the change with comment `vX.Y.Z`.
1. Run `cargo build --release`
1. Run `git tag vX.Y.Z`
1. Run `git push --tags upstream`
1. Run `( cd docs; ./build.sh )`
1. Run `cargo publish -p taskchampion`
1. Navigate to the tag in the GitHub releases UI and create a release with general comments about the changes in the release
1. Upload `./target/release/task` and `./target/release/task-sync-server` to the release
