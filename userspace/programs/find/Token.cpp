#include "Token.h"

BAN::Vector<Token> tokenize(const char* const* strings)
{
	constexpr Token type_map[] {
		{ Token::TOK_NOT,       "!"        },
		{ Token::TOK_LPAREN,    "("        },
		{ Token::TOK_RPAREN,    ")"        },
		{ Token::TOK_CURLIES,   "{}"       },
		{ Token::TOK_PLUS,      "+"        },
		{ Token::TOK_SEMICOLON, ";"        },
		{ Token::TOK_AND,       "-a"       },
		{ Token::TOK_OR,        "-o"       },
		{ Token::TOK_NAME,      "-name"    },
		{ Token::TOK_INAME,     "-iname"   },
		{ Token::TOK_PATH,      "-path"    },
		{ Token::TOK_NOUSER,    "-nouser"  },
		{ Token::TOK_NOGROUP,   "-nogroup" },
		{ Token::TOK_MOUNT,     "-mount"   },
		{ Token::TOK_XDEV,      "-xdev"    },
		{ Token::TOK_PRUNE,     "-prune"   },
		{ Token::TOK_PERM,      "-perm"    },
		{ Token::TOK_TYPE,      "-type"    },
		{ Token::TOK_LINKS,     "-links"   },
		{ Token::TOK_USER,      "-user"    },
		{ Token::TOK_GROUP,     "-group"   },
		{ Token::TOK_SIZE,      "-size"    },
		{ Token::TOK_ATIME,     "-atime"   },
		{ Token::TOK_CTIME,     "-ctime"   },
		{ Token::TOK_MTIME,     "-mtime"   },
		{ Token::TOK_EXEC,      "-exec"    },
		{ Token::TOK_OK,        "-ok"      },
		{ Token::TOK_PRINT,     "-print"   },
		{ Token::TOK_PRINT0,    "-print0"  },
		{ Token::TOK_NEWER,     "-newer"   },
		{ Token::TOK_DEPTH,     "-depth"   },
	};

	BAN::Vector<Token> tokens;

	for (size_t i = 0; strings[i]; i++)
	{
		Token::Type type { Token::TOK_LITERAL };
		for (const auto& entry : type_map)
		{
			if (strcmp(strings[i], entry.string) != 0)
				continue;
			type = entry.type;
			break;
		}
		MUST(tokens.push_back({ type, strings[i] }));
	}

	MUST(tokens.push_back({ Token::TOK_END, "<EOF>" }));

	return tokens;
}
