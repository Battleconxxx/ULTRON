#include <stdio.h>
#include <kernel/memory.h>

uint8_t memory_bitmap[MAX_FRAMES / 8];
memory_region_t usable_regions[MAX_MEMORY_REGIONS];
int usable_region_count = 0;


void parse_memory_map(multiboot_info_t* mbi) {
    printf("Inside parse_memory_map\n");
    if (!(mbi->flags & (1 << 6))) {
        printf("No memory map from GRUB\n");
        return;
    }

    uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;
    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbi->mmap_addr;
    printf("mbi->flags: 0x%x\n", mbi->flags);
    printf("mmap_addr: 0x%x\n", mbi->mmap_addr);
    printf("mmap_length: 0x%x\n", mbi->mmap_length);


    while ((uint32_t)mmap < mmap_end) {
        if (mmap->type == 1 && usable_region_count < MAX_MEMORY_REGIONS) {
            usable_regions[usable_region_count].base = mmap->addr;
            usable_regions[usable_region_count].length = mmap->len;
            usable_region_count++;

            printf("Usable memory map: base=0x%x, length=0x%x\n", (uint32_t)mmap->addr, (uint32_t)mmap->len);
        }
        mmap = (multiboot_memory_map_t*)((uintptr_t)mmap + mmap->size + sizeof(uint32_t));

    }
    printf("Number of usable regions: %d\n", usable_region_count);
}