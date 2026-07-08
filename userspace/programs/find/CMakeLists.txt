set(SOURCES
	main.cpp
	Node.cpp
	Token.cpp
)

add_executable(find ${SOURCES})
banan_link_library(find ban)
banan_link_library(find libc)

install(TARGETS find OPTIONAL)
