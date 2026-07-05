#!/bin/bash ../install.sh

NAME='libjpeg'
VERSION='10'
DOWNLOAD_URL="https://www.ijg.org/files/jpegsrc.v$VERSION.tar.gz#8b9eaa13242690ebd03e1728ab1edf97a81a78ed6e83624d493655f31ac95ab5"
TAR_CONTENT="jpeg-$VERSION"
CONFIG_SUB=('config.sub')
CONFIGURE_OPTIONS=(
	'--disable-static'
)
