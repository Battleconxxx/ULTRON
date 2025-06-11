#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>


#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002
#define MAX_MEMORY_REGIONS 32

typedef struct multiboot_memory_map {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) multiboot_memory_map_t;

typedef struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} multiboot_info_t;

typedef struct {
    uint64_t base;
    uint64_t length;
} memory_region_t;

void parse_memory_map(multiboot_info_t* mbi);

#define FRAME_SIZE 4096
#define TOTAL_MEMORY (4ULL * 1024 * 1024 * 1024)
#define MAX_FRAMES (TOTAL_MEMORY / FRAME_SIZE)
#define BITMAP_SIZE (MAX_FRAMES / 8)



#endif