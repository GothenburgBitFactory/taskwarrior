#! /bin/bash

REMOTE=origin

set -e

if ! [ -f "docs/src/SUMMARY.md" ]; then
    echo "Run this from the root of the repo"
    exit 1
fi

# build the latest version of the mdbook plugin
cargo build -p taskchampion-cli --features usage-docs --bin usage-docs

# create a worktree of this repo, with the `gh-pages` branch checked out
if ! [ -d ./docs/tmp ]; then
    git worktree add docs/tmp gh-pages
fi

# update the wortree
(cd docs/tmp && git pull $REMOTE gh-pages)

# remove all files in the worktree and regenerate the book there
rm -rf docs/tmp/*
mdbook build docs
cp -rp docs/book/* docs/tmp

# add everything in the worktree, commit, and push
(cd docs/tmp && git add -A)
(cd docs/tmp && git commit -am "update docs")
(cd docs/tmp && git push $REMOTE gh-pages:gh-pages)
