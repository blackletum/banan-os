#pragma once

#include <stdint.h>

#define BANAN_BOOTLOADER_MAGIC	0xD3C60CFF
#define BANAN_BOOTLOADER_FB_RGB		1
#define BANAN_BOOTLOADER_FB_TEXT	2

struct BananBootFramebufferInfo
{
	uint32_t address;
	uint32_t pitch;
	uint32_t width;
	uint32_t height;
	uint8_t bpp;
	uint8_t type;
};

struct BananBootloaderMemoryMapEntry
{
	uint64_t address;
	uint64_t length;
	uint32_t type;
	uint32_t unused;
};
static_assert(sizeof(BananBootloaderMemoryMapEntry) == 24);

struct BananBootloaderMemoryMapInfo
{
	uint32_t entry_count;
	uint32_t padding;
	struct BananBootloaderMemoryMapEntry entries[];
};
static_assert(sizeof(BananBootloaderMemoryMapInfo) == 8);

struct BananBootloaderInfo
{
	uint32_t command_line_addr;
	uint32_t framebuffer_addr;
	uint32_t memory_map_addr;
	uint32_t kernel_paddr;
};
static_assert(sizeof(BananBootloaderInfo) == 16);
