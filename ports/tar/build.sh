#!/bin/bash ../install.sh

NAME='tar'
VERSION='1.35'
DOWNLOAD_URL="https://ftpmirror.gnu.org/gnu/tar/tar-$VERSION.tar.xz#4d62ff37342ec7aed748535323930c7cf94acf71c3591882b26a7ea50f3edc16"
CONFIG_SUB=('build-aux/config.sub')

pre_configure() {
	echo '#include_next <sys/types.h>' > gnu/sys_types.in.h
}
