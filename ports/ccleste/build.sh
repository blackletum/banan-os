#!/bin/bash ../install.sh

NAME='ccleste'
VERSION='1.4.0'
DOWNLOAD_URL="https://github.com/lemon32767/ccleste/archive/refs/tags/v$VERSION.tar.gz#32dfd797f3c863201e0c19aa97974c56a8ed589a34c0522503f25f6e1399edd6"
DEPENDENCIES=('sdl2-compat' 'SDL2_mixer')

configure() {
	make clean
}

build() {
	make SDL_PREFIX="$BANAN_SYSROOT/usr/bin/" || exit 1
}

install() {
	mkdir -p "$DESTDIR/usr/bin" "$DESTDIR/usr/share/Games/ccleste"
	cp -v ccleste "$DESTDIR/usr/bin/" || exit 1
	cp -rv data/* "$DESTDIR/usr/share/Games/ccleste/" || exit 1
}
