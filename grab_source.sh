wget https://commondatastorage.googleapis.com/chromium-browser-official/chromium-121.0.6167.160.tar.xz
tar -xf chromium-121.0.6167.160.tar.xz --strip-components 1
git restore .gitignore readme.md
rm chromium-121.0.6167.160.tar.xz
