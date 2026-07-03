#pragma once

#include <stdint.h>

namespace Kernel::API
{

	enum SharedPageFeature : uint32_t
	{
		SPF_GETTIME = 1 << 0,
		SPF_RDTSCP  = 1 << 1,
	};

	struct SharedPage
	{
		uint16_t gdt_cpu_offset;

		uint32_t features;

		struct
		{
			uint64_t realtime_s;
			uint32_t realtime_ns;
		} gettime_shared;

		struct
		{
			struct
			{
				uint32_t seq;
				uint32_t mult;
				int8_t shift;
				uint64_t last_ns;
				uint64_t last_tsc;
			} gettime_local;
		} cpus[];
	};

}
