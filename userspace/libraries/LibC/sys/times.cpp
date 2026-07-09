#include <sys/times.h>
#include <time.h>
#include <unistd.h>

static inline clock_t timespec_to_clock_t(const timespec& ts)
{
	static clock_t clock_tick = -1;
	if (clock_tick == -1)
		clock_tick = sysconf(_SC_CLK_TCK);

	return ts.tv_sec * clock_tick + ts.tv_nsec * clock_tick / 1'000'000'000;
}

clock_t times(struct tms* buffer)
{
	// FIXME: system time, child times
	*buffer = {};

	timespec ts;

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
	buffer->tms_utime = timespec_to_clock_t(ts);

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return timespec_to_clock_t(ts);
}
