#!/bin/bash ../install.sh

NAME='openal-soft'
VERSION='1.25.2'
DOWNLOAD_URL="https://github.com/kcat/openal-soft/archive/refs/tags/$VERSION.tar.gz#fb27e5839aa11f0e5b9d33756965291fad5d6909ab928ea1f796f4a1a6877894"
DEPENDENCIES=('SDL3' 'zlib' 'libsndfile')

configure() {
	cmake --fresh -B build -S . -G Ninja \
		--toolchain="$BANAN_TOOLCHAIN_DIR/Toolchain.txt" \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DCMAKE_BUILD_TYPE=Release \
		-DALSOFT_EXAMPLES=OFF \
		-DALSOFT_NO_CONFIG_UTIL=ON \
		-DALSOFT_BACKEND_SDL3=ON \
		|| exit 1
}

build() {
	cmake --build build || exit 1
}

install() {
	DESTDIR="$DESTDIR" cmake --install build || exit 1
}
