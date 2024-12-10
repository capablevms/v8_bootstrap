wget https://commondatastorage.googleapis.com/chromium-browser-official/chromium-128.0.6613.114.tar.xz
tar -xf chromium-128.0.6613.114.tar.xz --strip-components 1
git restore .gitignore readme.md
rm chromium-128.0.6613.114.tar.xz
