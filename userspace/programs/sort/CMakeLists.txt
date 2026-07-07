set(SOURCES
	main.cpp
)

add_executable(sort ${SOURCES})
banan_link_library(sort ban)
banan_link_library(sort libc)

install(TARGETS sort OPTIONAL)
