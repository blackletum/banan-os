#pragma once

#include "Token.h"

#include <sys/types.h>
#include <time.h>

struct Node
{
	enum class Type
	{
		NOD_AND,    NOD_OR,      NOD_NOT,
		NOD_NAME,   NOD_INAME,   NOD_PATH,
		NOD_NOUSER, NOD_NOGROUP, NOD_MOUNT,
		NOD_XDEV,   NOD_PRUNE,   NOD_PERM,
		NOD_TYPE,   NOD_LINKS,   NOD_USER,
		NOD_GROUP,  NOD_SIZE,    NOD_ATIME,
		NOD_CTIME,  NOD_MTIME,   NOD_EXEC,
		NOD_OK,     NOD_PRINT,   NOD_PRINT0,
		NOD_NEWER,  NOD_DEPTH,
	};

	enum class Comp
	{
		Eq, Gt, Lt
	};

	Type type;
	union {
		// AND, OR
		struct {
			Node* lhs;
			Node* rhs;
		} bin_op;
		// NOT
		Node* child;
		// NAME, INAME, PATH
		const char* pattern;
		// PERM, TYPE
		mode_t mode;
		// USER
		uid_t uid;
		// GROUP
		gid_t gid;
		// LINKS, SIZE, ATIME, CTIME, MTIME
		struct {
			uint64_t value;
			Comp comp;
		} n;
		// EXEC, OK
		struct {
			const char* args;
			size_t count;
		} args;
		// NEWER
		timespec mtime;
	} as;

	Node(Type type, decltype(Node::as) as);
	Node(Type type);
};

extern bool g_mount_specified;
extern bool g_xdev_specified;
extern bool g_prune_specified;
extern bool g_depth_specified;

Node* compile_tokens(BAN::Span<const Token>);
bool evaluate_node(Node*, const char* path, struct stat&);
