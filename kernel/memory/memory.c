#include <stdio.h>
#include <string.h>
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

void set_frame(uint32_t frame) {
    memory_bitmap[frame / 8] |= (1 << (frame % 8));
}

void clear_frame(uint32_t frame) {
    memory_bitmap[frame / 8] &= ~(1 << (frame % 8));
}

int test_frame(uint32_t frame) {
    return memory_bitmap[frame / 8] & (1 << (frame % 8));
}

uint32_t first_free_frame() {
    for (uint32_t i = 0; i < MAX_FRAMES / 8; i++) {
        if (memory_bitmap[i] != 0xFF) {
            for (int j = 0; j < 8; j++) {
                if (!(memory_bitmap[i] & (1 << j))) {
                    return i * 8 + j;
                }
            }
        }
    }
    return (uint32_t)-1; // No free frames
}

extern uint32_t _start;
extern uint32_t __kernel_end;


void init_memory(){
    for(uint32_t i = 0; i < BITMAP_SIZE; i++){
        memory_bitmap[i] = 1;
    }
    printf("bitmap cleared\n");

    uint32_t kernel_start_addr = (uint32_t)&_start;
    uint32_t kernel_end_addr = (uint32_t)&__kernel_end;
    int count = 0;
    for (int i = 0; i < usable_region_count; i++) {
        uint32_t base = usable_regions[i].base;
        uint32_t length = usable_regions[i].length;
        uint32_t start_frame = base / FRAME_SIZE;
        uint32_t end_frame = (base + length + FRAME_SIZE -1) / FRAME_SIZE;

        
        for (uint32_t frame = start_frame; frame < end_frame; frame++) {

            uint32_t addr = frame * FRAME_SIZE;

            if (addr >= kernel_start_addr && addr < kernel_end_addr) continue;
            clear_frame(frame); // Mark as free
            count++;
        }
    }
    printf("Memory initialized with %d usable frames\n", count);
}

//heap

static uint8_t *heap_ptr;

void malloc_init(void){
    heap_ptr = __heap_start;
}

void *malloc(size_t size){
    if(!heap_ptr)malloc_init;

    if(heap_ptr + size > __heap_end){
        printf("malloc failed: out of heap memory (requested %u bytes)\n", size);
        return 0; // Or handle error
    }

    void* ptr = (void*)heap_ptr;
    heap_ptr+=size;
    return ptr;
}

void *calloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0) {
        return NULL;
    }
    if (size > (size_t)-1 / nmemb) {
        return NULL;  // overflow
    }
    size_t total = nmemb * size;
    void *p = malloc(total);
    if (!p) {
        return NULL;
    }
    memset(p, 0, total);
    return p;
}

void free(void *ptr) {
    (void)ptr;
    // no-op: memory is never reclaimed in this simple bump allocator
}