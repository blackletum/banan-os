set(SOURCES
	main.cpp
)

add_executable(top ${SOURCES})
banan_link_library(top ban)
banan_link_library(top libc)

install(TARGETS top OPTIONAL)
