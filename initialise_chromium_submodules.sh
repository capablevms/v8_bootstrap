# Note: the default branch of the repo in the first `git submodule add` determines the version of chromium that'll be
# set up. To change version, change the default branch.
# Hacky, I know, but it lets me move on for now.
# To update chromium to a new version:
# - Add a branch for that version in a chromium fork
# - Ensure that fork is linked below, and ensure that the new version's branch is the "default" branch for the repo
# - Ensure submodule links and revisions are correct for the repo
#   | Note that the history for chromium/third_party/webrtc is sometimes rewritten, making the revision used pinned by
#     an old version of Chromium out-of-date on occasion! Needs manually fixing.
#   | Also, make sure the URL for chromium/v8 points to our fork. Make sure that the default branch for our fork is
#     also the version that corresponds to the chromium release you're updating to. That's also a hacky fix, but also
#     keeps things simple at time of writing. We should fix this, but the usual dev workflow doesn't require updating
#     from the submodule's remote often (rsync instead) so it isn't important.
# - `rm -rf` your local chromium directory — or just move it somewhere if you want peace of mind — and run this script!
# - Once you know you're able to build v8 successfully, commit the changed submodules.
git submodule update --init --depth=1 --recommend-shallow chromium
cd chromium

git submodule set-url v8 git@github.com:probablytom/v8

# List of submodules we have to initialise. We avoid initialising everything to avoid pulling unneccesary deps below
for submod in 'third_party/angle' 'third_party/devtools-frontend/src' 'third_party/icu' 'third_party/perfetto' 'third_party/skia' 'third_party/vulkan-deps' 'third_party/webrtc' 'third_party/xdg-utils' 'v8';
do
	git submodule init $submod
done

# Back out to chromium
cd ..

# Grab chromium and up-to-date deps
git submodule update --recommend-shallow --recursive chromium

# Add freebsd-ports and apply patches
cp CMakeLists.txt chromium/CMakeLists.txt
cp -r cheribsd-patches chromium/cheribsd-patches
cd chromium
cmake .
make freebsd-ports
make apply-patches
make pre-configure
make configure-all

# System is now ready to kick off an initial build! (But we won't do that here, in case you need to inspect chromium)
