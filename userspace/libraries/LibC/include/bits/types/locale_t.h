#ifndef _BITS_TYPES_LOCALE_T_H
#define _BITS_TYPES_LOCALE_T_H 1

// https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/locale.h.html

#include <sys/cdefs.h>

__BEGIN_DECLS

typedef enum { __ENC_ASCII, __ENC_UTF8 } __encoding_e;
struct __locale_t
{
	const char* name;
	__encoding_e encoding;
};
typedef struct __locale_t* locale_t;

__END_DECLS

#endif
