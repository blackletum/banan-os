#include <BAN/Vector.h>
#include <BAN/String.h>
#include <BAN/Sort.h>

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct SortInfo
{
	enum class SortType {
		Default, Numeric,
	};

	SortType type { SortType::Default };

	bool reverse { false };
	bool unique  { false };

	bool only_blank_alnum { false };
	bool ignore_case      { false };
	bool ignore_nonprint  { false };

};

static const char* s_argv0 { nullptr };

static double get_numberic(const BAN::String& line)
{
	// FIXME: this isn't quite right :)
	errno = 0;
	const double value = strtod(line.data(), nullptr);
	return errno ? DBL_MAX : value;
}

static int compar(const BAN::String& lhs, const BAN::String& rhs, SortInfo info)
{
	const int mult = info.reverse ? -1 : 1;

	switch (info.type)
	{
		case SortInfo::SortType::Numeric:
		{
			const double lhs_value = get_numberic(lhs);
			const double rhs_value = get_numberic(rhs);
			if (lhs_value == rhs_value)
				return 0;
			return mult * (lhs_value < rhs_value ? -1 : 1);
		}
		case SortInfo::SortType::Default:
		{
			auto lhs_sv = lhs.sv().substring(0, lhs.size() - 1);
			auto rhs_sv = rhs.sv().substring(0, rhs.size() - 1);

			const auto skip_to_unignored = [&info](BAN::StringView& string) {
				while (info.only_blank_alnum && !string.empty() && !(isblank(string.front()) || isalnum(string.front())))
					string = string.substring(1);
				while (info.ignore_nonprint  && !string.empty() && !(isprint(string.front())))
					string = string.substring(1);
			};

			for (;;)
			{
				skip_to_unignored(lhs_sv);
				skip_to_unignored(rhs_sv);
				if (lhs_sv.empty() && rhs_sv.empty())
					return 0;
				if (lhs_sv.empty() || rhs_sv.empty())
					return mult * (lhs_sv.empty() ? -1 : 1);

				const int lhs_ch = info.ignore_case ? toupper(lhs_sv.front()) : lhs_sv.front();
				const int rhs_ch = info.ignore_case ? toupper(rhs_sv.front()) : rhs_sv.front();
				if (lhs_ch != rhs_ch)
					return mult * (lhs_ch < rhs_ch ? -1 : 1);

				lhs_sv = lhs_sv.substring(1);
				rhs_sv = rhs_sv.substring(1);
			}
		}
	}

	ASSERT_NOT_REACHED();
}

static BAN::Optional<BAN::String> read_next_line(const char* path, FILE* fp)
{
	BAN::String line;

	while (line.empty() || line.back() != '\n')
	{
		char buffer[128];
		if (fgets(buffer, sizeof(buffer), fp) == nullptr)
		{
			if (feof(fp))
				break;
			fprintf(stderr, "%s: fgets %s: %s\n", s_argv0, path, strerror(errno));
			return {};
		}

		if (auto ret = line.append(buffer); ret.is_error())
		{
			fprintf(stderr, "%s: malloc %s: %s\n", s_argv0, path, ret.error().get_message());
			return {};
		}
	}

	if (line.empty() || line.back() == '\n')
		return line;

	if (auto ret = line.push_back('\n'); ret.is_error())
	{
		fprintf(stderr, "%s: malloc %s: %s\n", s_argv0, path, ret.error().get_message());
		return {};
	}

	return line;
}

static bool do_sort(const char* out_path, const char** paths, size_t path_count, SortInfo info)
{
	BAN::Vector<BAN::String> lines;

	for (size_t i = 0; i < path_count; i++)
	{
		FILE* fp = strcmp(paths[i], "-") == 0 ? stdin : fopen(paths[i], "r");
		if (fp == nullptr)
		{
			fprintf(stderr, "%s: fopen %s: %s\n", s_argv0, paths[i], strerror(errno));
			return false;
		}

		for (;;)
		{
			auto opt_line = read_next_line(paths[i], fp);
			if (!opt_line.has_value())
				return false;
			if (opt_line->empty())
				break;

			if (auto ret = lines.push_back(opt_line.release_value()); ret.is_error())
			{
				fprintf(stderr, "%s: malloc %s: %s\n", s_argv0, paths[i], ret.error().get_message());
				return false;
			}
		}

		fclose(fp);
	}

	BAN::sort::sort(lines.begin(), lines.end(), [info](const auto& lhs, const auto& rhs) {
		return compar(lhs, rhs, info) <= 0;
	});

	FILE* out_fp = strcmp(out_path, "-") == 0 ? stdout : fopen(out_path, "w");
	if (out_fp == nullptr)
	{
		fprintf(stderr, "%s: fopen %s: %s\n", s_argv0, out_path, strerror(errno));
		return false;
	}

	for (size_t i = 0; i < lines.size(); i++)
	{
		if (info.unique && i != 0 && compar(lines[i - 1], lines[i], info) == 0)
			continue;

		size_t written = 0;
		while (written < lines[i].size())
		{
			const size_t nwrite = fwrite(lines[i].data() + written, 1, lines[i].size() - written, out_fp);
			if (nwrite == 0)
			{
				fprintf(stderr, "%s: fwrite %s: %s\n", s_argv0, out_path, strerror(errno));
				return false;
			}
			written += nwrite;
		}
	}

	fclose(out_fp);

	return true;
}

static bool do_merge(const char* out_path, const char** paths, size_t path_count, SortInfo info)
{
	// FIXME: optimize
	return do_sort(out_path, paths, path_count, info);
}

static bool do_check(const char** paths, size_t path_count, SortInfo info, bool silent)
{
	BAN::String prev_line;

	size_t line_idx = 0;

	for (size_t i = 0; i < path_count; i++)
	{
		FILE* fp = strcmp(paths[i], "-") == 0 ? stdin : fopen(paths[i], "r");
		if (fp == nullptr)
		{
			fprintf(stderr, "%s: fopen %s: %s\n", s_argv0, paths[i], strerror(errno));
			return false;
		}

		for (;;)
		{
			auto opt_line = read_next_line(paths[i], fp);
			if (!opt_line.has_value())
				return false;
			if (opt_line->empty())
				break;

			line_idx++;

			if (!prev_line.empty())
			{
				const int result = compar(prev_line, opt_line.value(), info);
				if (info.unique ? (result >= 0) : (result > 0))
				{
					if (!silent)
						fprintf(stderr, "%s: %s:%zu: disorder: %s", s_argv0, paths[i], line_idx, opt_line->data());
					return false;
				}
			}

			prev_line = opt_line.release_value();
		}

		fclose(fp);
	}

	return true;
}

int main(int argc, char** argv)
{
	s_argv0 = argv[0];

	enum class Operation {
		Sort, Merge, Check, CheckSilent,
	};

	Operation operation { Operation::Sort };
	const char* output = "-";
	SortInfo info {};

	for (;;)
	{
		static option long_options[] {
			{ "check",                 optional_argument, nullptr, 'c' },
			{ "merge",                 no_argument,       nullptr, 'm' },
			{ "output",                required_argument, nullptr, 'o' },
			{ "unique",                no_argument,       nullptr, 'u' },
			{ "reverse",               no_argument,       nullptr, 'r' },
			{ "dictionary-order",      no_argument,       nullptr, 'd' },
			{ "ignore-case",           no_argument,       nullptr, 'f' },
			{ "ignore-nonprinting",    no_argument,       nullptr, 'i' },
			{ "numeric-sort",          no_argument,       nullptr, 'n' },
			{ "help",                  no_argument,       nullptr,  0  },
			{}
		};

		int ch = getopt_long(argc, argv, "cCmo:urdfinbt", long_options, nullptr);
		if (ch == -1)
			break;

		switch (ch)
		{
			case 'c':
			case 'C':
				if (optarg == nullptr)
					operation = (ch == 'c') ? Operation::Check : Operation::CheckSilent;
				else if (strcmp(optarg, "silent") == 0 || strcmp(optarg, "quiet") == 0)
					operation = Operation::CheckSilent;
				else
				{
					fprintf(stderr, "%s: unknown argument '%s' for --check\n", argv[0], optarg);
					fprintf(stderr, "see '%s --help' for usage\n", argv[0]);
					return 1;
				}
				break;
			case 'm':
				operation = Operation::Merge;
				break;
			case 'o':
				output = optarg;
				break;
			case 'u':
				info.unique = true;
				break;
			case 'r':
				info.reverse = true;
				break;
			case 'd':
				info.only_blank_alnum = true;
				break;
			case 'f':
				info.ignore_case = true;
				break;
			case 'i':
				info.ignore_nonprint = true;
				break;
			case 'n':
				info.type = SortInfo::SortType::Numeric;
				break;
			case 0:
				fprintf(stderr, "usage: %s [OPTION]... [FILE]...\n", argv[0]);
				fprintf(stderr, "  sort and merge input FILE(s)\n");
				fprintf(stderr, "OPTIONS:\n");
				fprintf(stderr, "  -c, --check               check if input is sorted (and unique with -u) and print a warning message and exit accordingly\n");
				fprintf(stderr, "  -C, --check=silent|quiet  same as -c but don't print a warning message\n");
				fprintf(stderr, "  -m, --merge               merge only, input is assumed to be sorted\n");
				fprintf(stderr, "  -o, --output=FILE         output into FILE instead of stdout\n");
				fprintf(stderr, "  -u, --unique              remove all but one line with equal keys\n");
				fprintf(stderr, "  -r, --reverse             reverse the result of output\n");
				fprintf(stderr, "  -d, --directory-order     sort only according to blank and alphanumeric characters\n");
				fprintf(stderr, "  -f, --ignore-case         compare using upper case variants of each character\n");
				fprintf(stderr, "  -i, --ignore-nonprinting  sort only according to printable characters\n");
				fprintf(stderr, "  -n, --numeric-sort        sort based on numberic values optionally preceded by blanks\n");
				fprintf(stderr, "      --help                show this message and exit\n");
				return 0;
			case ':' : case '?':
				fprintf(stderr, "see '%s --help' for usage\n", argv[0]);
				return 1;
		}
	}

	const char* fallback_in = "-";

	const char* const* paths = (optind == argc) ? &fallback_in : &argv[optind];
	const size_t path_count = (optind == argc) ? 1 : argc - optind;

	bool success;
	switch (operation)
	{
		case Operation::Sort:
			success = do_sort(output, const_cast<const char**>(paths), path_count, info);
			break;
		case Operation::Merge:
			success = do_merge(output, const_cast<const char**>(paths), path_count, info);
			break;
		case Operation::Check:
		case Operation::CheckSilent:
			success = do_check(const_cast<const char**>(paths), path_count, info, operation == Operation::CheckSilent);
			break;
	}

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
