#!/bin/bash ../install.sh

NAME='freetype'
VERSION='2.14.3'
DOWNLOAD_URL="https://download.savannah.gnu.org/releases/freetype/freetype-$VERSION.tar.xz#36bc4f1cc413335368ee656c42afca65c5a3987e8768cc28cf11ba775e785a5f"
DEPENDENCIES=('zlib' 'libpng')
CONFIGURE_OPTIONS=(
	'--disable-static'
	'lt_cv_deplibs_check_method=pass_all'
)
