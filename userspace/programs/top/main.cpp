#include <BAN/HashMap.h>
#include <BAN/Sort.h>
#include <BAN/String.h>
#include <BAN/Vector.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/banan-os.h>
#include <termios.h>
#include <unistd.h>

winsize g_winsize;

struct ProcessInfo
{
	pid_t pid;
	BAN::String virt;
	BAN::String phys;
	uint32_t cpu_load;
	BAN::String command;
};
BAN::Vector<ProcessInfo> g_process_infos;
BAN::HashMap<pid_t, uint64_t> g_process_prev_us;

struct MemInfo
{
	size_t total_kib;
	size_t free_kib;
	size_t used_kib;
};
MemInfo g_meminfo;

void update_process_info(uint64_t delta_ms)
{
	g_process_infos.clear();
	const auto prev_us = BAN::move(g_process_prev_us);

	DIR* dirp = opendir("/proc");
	if (dirp == nullptr)
		return;

	const dirent* dent;
	while ((dent = readdir(dirp)))
	{
		char* endp;
		const pid_t pid = strtol(dent->d_name, &endp, 10);
		if (*endp)
			continue;

		int pid_fd = openat(dirfd(dirp), dent->d_name, O_RDONLY);
		if (pid_fd == -1)
			continue;

		ProcessInfo info {};
		info.pid = pid;

		const auto read_from = [pid_fd](const char* file, void* buffer, size_t size) {
			int fd = openat(pid_fd, file, O_RDONLY);
			ASSERT(fd >= 0);
			read(fd, buffer, size);
			close(fd);
		};

		{
			const auto bytes_to_string = [](size_t size) -> BAN::String {
				if (size < 1024)
					return MUST(BAN::String::formatted("{}", size));

				size = size / 1024 * 10;

				size_t suffix_idx = 0;
				for (; size >= 10240; size /= 1024)
					suffix_idx++;

				constexpr char suffix[] { 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y', 'R', 'Q' };
				if (size >= 100)
					return MUST(BAN::String::formatted("{}{}", size / 10, suffix[suffix_idx]));
				return MUST(BAN::String::formatted("{}.{}{}", size / 10, size % 10, suffix[suffix_idx]));
			};

			proc_meminfo_t meminfo;
			read_from("meminfo", &meminfo, sizeof(meminfo));
			info.virt = bytes_to_string(meminfo.virt_pages * meminfo.page_size);
			info.phys = bytes_to_string(meminfo.phys_pages * meminfo.page_size);
		}

		{
			char buffer[32];
			read_from("cputime", buffer, sizeof(buffer));
			buffer[sizeof(buffer) - 1] = '\0';

			const auto cpu_us = strtoull(buffer, nullptr, 10) / 1000;

			uint64_t cpu_delta_us = cpu_us;
			if (auto it = prev_us.find(pid); it != prev_us.end())
				cpu_delta_us -= it->value;
			info.cpu_load = cpu_delta_us / delta_ms;

			MUST(g_process_prev_us.insert(pid, cpu_us));
		}

		{
			char buffer[128];
			read_from("cmdline", buffer, sizeof(buffer));
			buffer[sizeof(buffer) - 1] = '\0';
			info.command = buffer;
		}

		MUST(g_process_infos.push_back(BAN::move(info)));

		close(pid_fd);
	}

	closedir(dirp);

	BAN::sort::sort(g_process_infos.begin(), g_process_infos.end(), [](auto& a, auto& b) {
		return a.cpu_load > b.cpu_load;
	});
}

void update_info(uint64_t delta_ms)
{
	update_process_info(delta_ms);

	{
		int fd = open("/proc/meminfo", O_RDONLY);
		ASSERT(fd >= 0);
		full_meminfo_t meminfo;
		read(fd, &meminfo, sizeof(meminfo));
		close(fd);

		g_meminfo.total_kib = (meminfo.free_pages + meminfo.used_pages) * meminfo.page_size / 1024;
		g_meminfo.free_kib  =  meminfo.free_pages                       * meminfo.page_size / 1024;
		g_meminfo.used_kib  =  meminfo.used_pages                       * meminfo.page_size / 1024;
	}
}

void render_info()
{
	size_t header_rows = 0;

	printf("\e[%zuHProcesses: %zu total\e[K",
		++header_rows,
		g_process_infos.size()
	);

	printf("\e[%zuHMemory (MiB): %zu.%03zu total, %zu.%03zu free, %zu.%01zu used\e[K",
		++header_rows,
		g_meminfo.total_kib / 1024, g_meminfo.total_kib % 1024 * 1000 / 1024,
		g_meminfo.free_kib  / 1024, g_meminfo.free_kib  % 1024 * 1000 / 1024,
		g_meminfo.used_kib  / 1024, g_meminfo.used_kib  % 1024 * 1000 / 1024
	);

	printf("\e[%zuH\e[K",
		++header_rows
	);

	printf("\e[%zuH\e[7m  PID  VIRT  PHYS  %%CPU COMMAND\e[K\e[27m",
		++header_rows
	);

	const size_t count = BAN::Math::min<size_t>(g_winsize.ws_row - header_rows, g_process_infos.size());
	for (size_t i = 0; i < count; i++)
	{
		const auto& info = g_process_infos[i];
		printf("\e[%zuH", i + header_rows + 1);
		printf("%5d %5s %5s %3u.%01u %s",
			info.pid,
			info.virt.data(),
			info.phys.data(),
			info.cpu_load / 10, info.cpu_load % 10,
			info.command.data()
		);
		printf("\e[K");
	}

	printf("\e[J");

	fflush(stdout);
}

int main()
{
	signal(SIGWINCH, [](int) { tcgetwinsize(STDOUT_FILENO, &g_winsize); });
	if (tcgetwinsize(STDOUT_FILENO, &g_winsize) == -1)
	{
		fprintf(stderr, "could not get size of STDOUT\n");
		return 1;
	}

	static volatile bool is_running = true;
	signal(SIGINT, [](int) { is_running = false; });

	const auto get_current_ms = []() -> uint64_t {
		timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return ts.tv_sec * 1000 + ts.tv_nsec / 1'000'000;
	};

	constexpr uint64_t update_freq_ms = 1000;

	printf("\e[?25l"); fflush(stdout);

	auto prev_update_ms = 0;
	while (is_running)
	{
		const auto current_ms = get_current_ms();
		if (current_ms >= prev_update_ms + update_freq_ms)
		{
			update_info(current_ms - prev_update_ms);

			prev_update_ms += update_freq_ms;
			if (prev_update_ms + update_freq_ms < current_ms)
				prev_update_ms = current_ms;
		}

		render_info();

		const auto timeout_ms = (prev_update_ms + update_freq_ms) - current_ms;
		const timespec timeout_ts {
			.tv_sec  = static_cast<time_t>((timeout_ms / 1000)),
			.tv_nsec = static_cast<long>((timeout_ms % 1000) * 1'000'000)
		};
		nanosleep(&timeout_ts, nullptr);
	}

	printf("\e[?25h\n");
}
