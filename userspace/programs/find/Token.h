#pragma once

#include <BAN/Vector.h>

struct Token
{
	enum Type
	{
		TOK_NOT,     TOK_LPAREN,    TOK_RPAREN, TOK_CURLIES,
		TOK_PLUS,    TOK_SEMICOLON, TOK_AND,    TOK_OR,
		TOK_NAME,    TOK_INAME,     TOK_PATH,   TOK_NOUSER,
		TOK_NOGROUP, TOK_MOUNT,     TOK_XDEV,   TOK_PRUNE,
		TOK_PERM,    TOK_TYPE,      TOK_LINKS,  TOK_USER,
		TOK_GROUP,   TOK_SIZE,      TOK_ATIME,  TOK_CTIME,
		TOK_MTIME,   TOK_EXEC,      TOK_OK,     TOK_PRINT,
		TOK_PRINT0,  TOK_NEWER,     TOK_DEPTH,  TOK_LITERAL,
		TOK_END
	};

	Type type;
	const char* string;
};

BAN::Vector<Token> tokenize(const char* const* strings);
