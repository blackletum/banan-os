#include <sys/auxv.h>
#include <unistd.h>

#include <kernel/API/SharedPage.h>

struct auxv
{
	unsigned long a_type;
	unsigned long a_val;
};

static const auxv* s_auxv = nullptr;

volatile Kernel::API::SharedPage* g_shared_page = nullptr;

__attribute__((constructor(101)))
static void _init_auxv()
{
	if (environ == nullptr)
		return;

	const char* const* null_env = environ;
	while (*null_env)
		null_env++;

	s_auxv = reinterpret_cast<const auxv*>(null_env + 1);

	if (auto value = getauxval(AT_SHARED_PAGE))
		g_shared_page = reinterpret_cast<volatile Kernel::API::SharedPage*>(value);
}

unsigned long getauxval(unsigned long type)
{
	if (s_auxv == nullptr)
		return 0;
	for (const auxv* aux = s_auxv; aux->a_type != AT_NULL; aux++)
		if (aux->a_type == type)
			return aux->a_val;
	return 0;
}
