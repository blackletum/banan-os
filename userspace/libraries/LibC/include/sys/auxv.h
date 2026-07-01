#ifndef _SYS_AUXV_H
#define _SYS_AUXV_H 1

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <elf.h>

unsigned long getauxval(unsigned long type);

__END_DECLS

#endif
