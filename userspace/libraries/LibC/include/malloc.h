#ifndef _MALLOC_H
#define _MALLOC_H 1

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __need_size_t
#include <stddef.h>

struct mallinfo
{
	int arena;
	int ordblks;
	int smblks;
	int hblks;
	int hblkhd;
	int usmblks;
	int fsmblks;
	int uordblks;
	int fordblks;
	int keepcost;
};

struct mallinfo2
{
	size_t arena;
	size_t ordblks;
	size_t smblks;
	size_t hblks;
	size_t hblkhd;
	size_t usmblks;
	size_t fsmblks;
	size_t uordblks;
	size_t fordblks;
	size_t keepcost;
};

void* malloc(size_t total_size);
void  free(void* ptr);
void* realloc(void* ptr, size_t size);
void* calloc(size_t nmemb, size_t size);
void* reallocarray(void* ptr, size_t nmemb, size_t size);
void* aligned_alloc(size_t alignment, size_t size);
int   posix_memalign(void** memptr, size_t alignment, size_t size);

size_t malloc_usable_size(void* ptr);

struct mallinfo mallinfo(void);
struct mallinfo2 mallinfo2(void);

__END_DECLS

#endif
