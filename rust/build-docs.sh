#! /bin/bash

set -x

REMOTE=origin

set -e

if ! [ -f "docs/src/SUMMARY.md" ]; then
    echo "Run this from the root of the repo"
    exit 1
fi

# create a worktree of this repo, with the `gh-pages` branch checked out
git branch -f gh-pages $REMOTE/gh-pages
if ! [ -d ./docs/tmp ]; then
    git worktree add -f docs/tmp gh-pages
fi

# update the wortree
(cd docs/tmp && git pull $REMOTE gh-pages)

# remove all files in the worktree and regenerate the book there
git worktree remove -f docs/tmp
rm -rf docs/tmp/*
mdbook build docs
mkdir docs/tmp
cp -rp docs/book/* docs/tmp

# add everything in the worktree, commit, and push
(cd docs/tmp && git add -A)
(cd docs/tmp && git commit -am "update docs")
(cd docs/tmp && git push $REMOTE gh-pages:gh-pages)
