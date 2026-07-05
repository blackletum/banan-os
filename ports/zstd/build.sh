#!/bin/bash ../install.sh

NAME='zstd'
VERSION='1.5.7'
DOWNLOAD_URL="https://github.com/facebook/zstd/releases/download/v$VERSION/zstd-$VERSION.tar.gz#eb33e51f49a15e023950cd7825ca74a4a2b43db8354825ac24fc1b7ee09e6fa3"

configure() {
	cmake --fresh -S build/cmake -B _build -G Ninja \
		--toolchain="$BANAN_TOOLCHAIN_DIR/Toolchain.txt" \
		-DCMAKE_INSTALL_PREFIX='/usr' \
		-DCMAKE_BUILD_TYPE=Release \
		-DBUILD_SHARED_LIBS=ON \
		-DZSTD_PROGRAMS_LINK_SHARED=ON \
		-DZSTD_BUILD_SHARED=ON \
		-DZSTD_BUILD_STATIC=OFF \
		 || exit 1
}

build() {
	cmake --build _build ||exit 1
}

install() {
	DESTDIR="$DESTDIR" cmake --install _build ||exit 1
}
