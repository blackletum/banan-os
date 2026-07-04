#!/bin/bash ../install.sh

NAME='ncurses'
VERSION='6.6'
DOWNLOAD_URL="https://ftpmirror.gnu.org/gnu/ncurses/ncurses-$VERSION.tar.gz#355b4cbbed880b0381a04c46617b7656e362585d52e9cf84a67e2009b749ff11"
CONFIG_SUB=('config.sub')
CONFIGURE_OPTIONS=(
	"--with-pkg-config='$PKG_CONFIG'"
	"--with-pkg-config-libdir=/usr/lib/pkgconfig"
	'--enable-pc-files'
	'--enable-sigwinch'
	'--disable-widec'
	'--with-shared'
	'--with-trace'
	'--without-ada'
	'--without-manpages'
	CFLAGS='-std=c17'
)

post_install() {
	shellrc="$BANAN_SYSROOT/home/user/.shellrc"
	grep -q 'export NCURSES_NO_UTF8_ACS=' "$shellrc" || echo 'export NCURSES_NO_UTF8_ACS=1' >> "$shellrc"
}
