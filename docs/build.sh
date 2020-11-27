#! /bin/bash

REMOTE=origin

set -e

if ! [ -f "./src/SUMMARY.md" ]; then
    echo "Run this from the docs/ dir"
    exit 1
fi

if ! [ -d ./tmp ]; then
    git worktree add tmp gh-pages
fi

(cd tmp && git pull $REMOTE gh-pages)

rm -rf tmp/*
mdbook build
cp -rp book/* tmp
(cd tmp && git add -A)
(cd tmp && git commit -am "update docs")
(cd tmp && git push $REMOTE gh-pages:gh-pages)
