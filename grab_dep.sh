#!/bin/bash
# Tool to set up dependancies at the right commit.
# To be run from chromium's dir.
# Takes 1 arg: the path to the dependancy.
#   - Anything that previously existed at that path should be deleted.
# Will construct a submodule at _almost_ the correct revision
#   - (It'll be a commit ahead --- TODO fix...!)
TIMESTAMP=$(git show --no-patch --format=%ci $(git show-ref -s origin/HEAD))

git submodule add --depth 1 https://chromium.googlesource.com/chromium/src/$1 $1
cd $1
git fetch --shallow-since "${TIMESTAMP}" origin main
git checkout `git rev-list --max-parents=0 HEAD | tail -n 1`
