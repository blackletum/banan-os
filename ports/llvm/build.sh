#!/bin/bash ../install.sh

NAME='llvm'
VERSION='22.1.8'
DOWNLOAD_URL="https://github.com/llvm/llvm-project/releases/download/llvmorg-$VERSION/llvm-project-$VERSION.src.tar.xz#922f1817a0df7b1489272d18134ee0087a8b068828f87ac63b9861b1a9965888"
TAR_CONTENT="llvm-project-$VERSION.src"
DEPENDENCIES=('zlib' 'zstd')

configure() {
	unset CC CXX LD

	cmake --fresh -B build -S llvm -G Ninja \
		--toolchain="$BANAN_TOOLCHAIN_DIR/Toolchain.txt" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DLLVM_ENABLE_PROJECTS= \
		-DLLVM_ENABLE_RTTI=ON \
		-DLLVM_TARGETS_TO_BUILD=X86 \
		-DLLVM_INCLUDE_BENCHMARKS=OFF \
		-DLLVM_INCLUDE_TESTS=OFF \
		-DLLVM_HOST_TRIPLE=x86_64-pc-banan_os \
		-DLLVM_PARALLEL_LINK_JOBS=1 \
		 || exit 1
}

build() {
	cmake --build build || exit 1
}

install() {
	# This port only contains llvm libraries used optionally by
	# mesa port. There is no need to install and fill the disk :D
	:
}
