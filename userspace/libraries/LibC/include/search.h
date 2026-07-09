#ifndef _SEARCH_H
#define _SEARCH_H 1

// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/search.h.html

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __need_size_t
#include <sys/types.h>

typedef struct
{
	char* key;
	void* data;
} ENTRY;

typedef enum { FIND, ENTER } ACTION;
typedef enum { preorder, postorder, endorder, leaf } VISIT;

// NOTE: POSIX requires posix_tnode to be void but
//       defining an internal type avoids casting
#ifdef __is_libc
typedef struct __posix_tnode posix_tnode;
#else
typedef void posix_tnode;
#endif

int          hcreate(size_t nel);
void         hdestroy(void);
ENTRY*       hsearch(ENTRY item, ACTION action);
void         insque(void* element, void* pred);
void*        lfind(const void* key, const void* base, size_t* nelp, size_t width, int (*compar)(const void*, const void*));
void*        lsearch(const void* key, void* base, size_t* nelp, size_t width, int (*compar)(const void*, const void*));
void         remque(void* element);
void*        tdelete(const void* __restrict key, posix_tnode** __restrict rootp, int (*compar)(const void*, const void*));
posix_tnode* tfind(const void* key, posix_tnode* const* rootp, int (*compar)(const void*, const void*));
posix_tnode* tsearch(const void* key, posix_tnode** rootp, int (*compar)(const void*, const void*));
void         twalk(const posix_tnode* root, void (*action)(const posix_tnode*, VISIT, int));

__END_DECLS

#endif
