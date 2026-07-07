#!/bin/bash ../install.sh

NAME='xbanan'
VERSION='git'
DOWNLOAD_URL="https://git.bananymous.com/Bananymous/xbanan.git#b03a10d6012fc5504d98b9a5a6cab75779193e79"
DEPENDENCIES=('xorgproto')

configure() {
	cmake --fresh -B build -S . -G Ninja \
		--toolchain="$BANAN_TOOLCHAIN_DIR/Toolchain.txt" \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DCMAKE_BUILD_TYPE=Release \
		-DPLATFORM=banan-os \
		-DFONT_PATH=/usr/share/fonts/X11 \
		|| exit 1
}

build() {
	cmake --build build --target xbanan || exit 1
}

install() {
	mkdir -p "$DESTDIR/usr/bin"
	cp -v build/xbanan/xbanan "$DESTDIR/usr/bin/" || exit 1

	mkdir -p "$DESTDIR/usr/share/fonts/X11"
	cp -r fonts/misc "$DESTDIR/usr/share/fonts/X11/" || exit 1
}

post_install() {
	shellrc="$BANAN_SYSROOT/home/user/.shellrc"
	grep -q 'export DISPLAY=' "$shellrc" || echo 'export DISPLAY=:69' >> "$shellrc"
}
