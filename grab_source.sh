wget https://commondatastorage.googleapis.com/chromium-browser-official/chromium-122.0.6261.69.tar.xz
tar -xf chromium-122.0.6261.69.tar.xz --strip-components 1
git restore .gitignore readme.md
rm chromium-122.0.6261.69.tar.xz
