#include "Node.h"
#include "Token.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

enum class LinkMode
{
	Physical, Logical, Half
};

const char* g_argv0 { nullptr };

static dev_t s_first_st_dev { 0 };

static void recurse_path(int fd, const char* name, const char* path, Node* node, LinkMode link_mode)
{
	int stat_flag = 0;
	if (link_mode == LinkMode::Physical)
		stat_flag = AT_SYMLINK_NOFOLLOW;
	else if (link_mode == LinkMode::Half && (fd != AT_FDCWD))
		stat_flag = AT_SYMLINK_NOFOLLOW;

	struct stat st;
	if (fstatat(fd, name, &st, stat_flag) == -1)
	{
		fprintf(stderr, "%s: '%s': %s\n", g_argv0, path, strerror(errno));
		return;
	}

	if (fd == AT_FDCWD)
		s_first_st_dev = st.st_dev;

	if (g_mount_specified && st.st_dev != s_first_st_dev)
		return;

	if (!g_depth_specified || !S_ISDIR(st.st_mode))
		evaluate_node(node, path, st);

	if (S_ISDIR(st.st_mode) && (!g_prune_specified || g_depth_specified) && (!g_xdev_specified || st.st_dev == s_first_st_dev))
	{
		int open_flag = stat_flag ? O_NOFOLLOW : 0;

		int dirfd = openat(fd, name, O_RDONLY | O_DIRECTORY | open_flag);
		if (dirfd < 0)
			fprintf(stderr, "%s: '%s': %s\n", g_argv0, path, strerror(errno));

		DIR* dirp = nullptr;
		if ((dirfd >= 0) && (dirp = fdopendir(dirfd)))
		{
			char buffer[PATH_MAX];

			char* buffer_end = stpcpy(buffer, path);
			if (*path && buffer_end[-1] != '/')
				*buffer_end++ = '/';

			dirent* dirent;
			while ((errno = 0) || (dirent = readdir(dirp)))
			{
				if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0)
					continue;
				strcpy(buffer_end, dirent->d_name);
				recurse_path(dirfd, dirent->d_name, buffer, node, link_mode);
			}

			if (errno)
				fprintf(stderr, "%s: '%s': %s\n", g_argv0, path, strerror(errno));

			closedir(dirp);
		}

		if (dirp == nullptr && dirfd >= 0)
		{
			fprintf(stderr, "%s: '%s': %s\n", g_argv0, path, strerror(errno));
			close(dirfd);
		}
	}

	if (g_depth_specified && S_ISDIR(st.st_mode))
		evaluate_node(node, path, st);
}

int main(int argc, char** argv)
{
	g_argv0 = argv[0];

	LinkMode link_mode { LinkMode::Physical };

	int idx = 1;
	for (; idx < argc; idx++)
	{
		if (argv[idx][0] != '-')
			break;
		else if (strcmp(argv[idx], "-P") == 0)
			link_mode = LinkMode::Physical;
		else if (strcmp(argv[idx], "-L") == 0)
			link_mode = LinkMode::Logical;
		else if (strcmp(argv[idx], "-H") == 0)
			link_mode = LinkMode::Half;
		else if (strcmp(argv[idx], "--help") == 0)
		{
			fprintf(stderr, "usage: %s [-P|-L|-H] PATH... [EXPRESSION...]\n", argv[0]);
			fprintf(stderr, "  find files based on expressions\n");
			fprintf(stderr, "OPTIONS:\n");
			fprintf(stderr, "  -P          never follow symbolic links\n");
			fprintf(stderr, "  -L          follow symbolic links\n");
			fprintf(stderr, "  -H          only follow symbolic links in PATH...\n");
			fprintf(stderr, "      --help  show this message and exit\n");
			return 0;
		}
		else break;
	}

	if (idx < argc && strcmp(argv[idx], "--") == 0)
		idx++;

	constexpr auto is_path_start = [](char ch) {
		return ch != '-' && ch != '!' && ch != '(';
	};

	size_t path_count = 0;
	while (idx + path_count < static_cast<size_t>(argc) && is_path_start(argv[idx + path_count][0]))
		path_count++;

	auto tokens = tokenize(&argv[idx + path_count]);
	auto* node = compile_tokens(tokens.span());

	if (path_count == 0)
		recurse_path(AT_FDCWD, ".", ".", node, link_mode);
	else for (size_t i = 0; i < path_count; i++)
		recurse_path(AT_FDCWD, argv[idx + i], argv[idx + i], node, link_mode);

	return 0;
}
