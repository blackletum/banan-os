#!/bin/bash ../install.sh

NAME='butterscotch'
VERSION='git'
DOWNLOAD_URL="https://github.com/ButterscotchRunner/Butterscotch.git#c40c17418a3aee6c75d3dcd71a5ce0c8eb258b8e"
DEPENDENCIES=('SDL3' 'openal-soft')

configure() {
	cmake --fresh -B build -S . -G Ninja \
		--toolchain="$BANAN_TOOLCHAIN_DIR/Toolchain.txt" \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DCMAKE_BUILD_TYPE=Release \
		-DPLATFORM='desktop' \
		-DAUDIO_BACKEND='openal' \
		-DDESKTOP_BACKEND='sdl3' \
		. || exit 1
}

build() {
	cmake --build build || exit 1
}

install() {
	mkdir -p "$DESTDIR/usr/bin" || exit 1
	cp -vf build/butterscotch "$DESTDIR/usr/bin/" || exit 1
}
