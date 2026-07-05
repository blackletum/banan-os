#!/bin/bash ../install.sh

NAME='libspng'
VERSION='0.7.4'
DOWNLOAD_URL="https://github.com/randy408/libspng/archive/v0.7.4.tar.gz#47ec02be6c0a6323044600a9221b049f63e1953faf816903e7383d4dc4234487"
DEPENDENCIES=('zlib')
CONFIGURE_OPTIONS=(
	'-Dprefix=/usr'
	'-Dbuildtype=release'
	'-Dbuild_examples=false'
)

configure() {
	meson setup \
		--reconfigure \
		--cross-file "$MESON_CROSS_FILE" \
		"${CONFIGURE_OPTIONS[@]}" \
		build || exit 1
}

build() {
	meson compile -C build || exit 1
}

install() {
	meson install --destdir="$DESTDIR" -C build || exit 1
}
