#!/bin/bash ../install.sh

NAME='lynx'
VERSION='2.9.3'
DOWNLOAD_URL="https://invisible-island.net/archives/lynx/tarballs/lynx$VERSION.tar.gz#6e99e46980974a6d89eceefbb26ca8c7aa7702b78ecb5bad383b859af225d052"
TAR_CONTENT="lynx$VERSION"
DEPENDENCIES=('ncurses' 'openssl' 'libiconv' 'bzip2' 'zlib' 'zstd')
CONFIGURE_OPTIONS=(
	'--with-screen=ncurses'
	'--with-ssl'
)
