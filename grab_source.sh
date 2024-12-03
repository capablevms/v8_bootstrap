wget https://commondatastorage.googleapis.com/chromium-browser-official/chromium-126.0.6478.127.tar.xz
tar -xf chromium-126.0.6478.127.tar.xz --strip-components 1
git restore .gitignore readme.md
rm chromium-126.0.6478.127.tar.xz
