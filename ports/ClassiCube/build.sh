#!/bin/bash ../install.sh

NAME='ClassiCube'
VERSION='1.3.8'
DOWNLOAD_URL="https://github.com/ClassiCube/ClassiCube/archive/refs/tags/$VERSION.tar.gz#35293acf1e63baeca832dec2512283f2975c79ddf80cc855a12c10464723a6c4"
DEPENDENCIES=('SDL3' 'openal-soft' 'openssl' 'dejavu-fonts-ttf')

configure() {
	make clean PLAT=banan-os
}

build() {
	make -j$(nproc) banan-os RELEASE=1 || exit 1
}

install() {
	mkdir -p "$DESTDIR/usr/bin"
	cp -v 'ClassiCube' "$DESTDIR/usr/bin/" || exit 1
}
