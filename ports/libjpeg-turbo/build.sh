#!/bin/bash ../install.sh

NAME='libjpeg-turbo'
VERSION='3.2.0'
DOWNLOAD_URL="https://github.com/libjpeg-turbo/libjpeg-turbo/releases/download/$VERSION/libjpeg-turbo-$VERSION.tar.gz#6f30092cef9fb839779646608f4ee14ae3cbac989c47fa05e841b0841f09878e"
DEPENDENCIES=('zlib' 'libspng')

configure() {
	cmake --fresh -S . -B build -G Ninja \
		--toolchain="$BANAN_TOOLCHAIN_DIR/Toolchain.txt" \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DCMAKE_BUILD_TYPE=Release \
		-DENABLE_STATIC=OFF \
		-DWITH_SYSTEM_ZLIB=ON \
		-DWITH_SYSTEM_SPNG=ON \
		-DWITH_JPEG8=ON \
		-DWITH_TESTS=OFF \
		 || exit 1
}

build() {
	cmake --build build || exit 1
}

install() {
	DESTDIR="$DESTDIR" cmake --install build || exit 1
}
