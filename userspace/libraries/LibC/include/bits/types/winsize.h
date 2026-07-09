#ifndef _BITS_TYPES_WINSIZE_H
#define _BITS_TYPES_WINSIZE_H 1

// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/termios.h.html

#include <sys/cdefs.h>

__BEGIN_DECLS

struct winsize
{
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel; /* unused by kernel */
	unsigned short ws_ypixel; /* unused by kernel */
};

__END_DECLS

#endif
