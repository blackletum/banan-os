#!/bin/bash ../install.sh

NAME='dejavu-fonts-ttf'
VERSION='2.37'
DOWNLOAD_URL="http://sourceforge.net/projects/dejavu/files/dejavu/$VERSION/dejavu-fonts-ttf-$VERSION.tar.bz2#fa9ca4d13871dd122f61258a80d01751d603b4d3ee14095d65453b4e846e17d7"

configure() {
	:
}

build() {
	:
}

install() {
	mkdir -p "$DESTDIR/usr/share/fonts/TTF" || exit 1
	cp -v ttf/* "$DESTDIR/usr/share/fonts/TTF/" || exit 1

	mkdir -p "$DESTDIR/usr/share/fontconfig/conf.avail" || exit 1
	cp -v "fontconfig/"* "$DESTDIR/usr/share/fontconfig/conf.avail/" || exit 1
}
