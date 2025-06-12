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

//Paging

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

void init_paging(){

    for(int i=0; i< 1024; i++)page_directory[i] = 0;

    static uint32_t identity_tables[4][1024] __attribute__((aligned(4096)));
    for(int table = 0; table<4; table++){
        for(int i=0; i<1024; i++){
            identity_tables[table][i] = ((table * 0x400000) + (i * 0x1000)) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }
        page_directory[table] = ((uint32_t)identity_tables[table]) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }

    //reserve the first 4096 frames
    // ✅ Reserve first 16 MiB in the frame bitmap (4096 frames)
    for (uint32_t i = 0; i < 4096; i++) {
        set_frame(i);
    }

    // Map kernel higher-half: 0xC0000000 - 0xC03FFFFF (maps 0x00100000 physical)
    static uint32_t kernel_table[1024] __attribute__((aligned(4096)));
    for(int i=0; i<1024; i++){
        kernel_table[i] = (i * 0x1000 + 0x00100000) | PAGE_PRESENT | PAGE_RW; 
    }
    page_directory[KERNEL_VIRTUAL_BASE >> 22] = ((uint32_t)kernel_table) | PAGE_PRESENT | PAGE_RW;

    page_directory[1023] = ((uint32_t)page_directory) | PAGE_PRESENT | PAGE_RW;


        // Reserve kernel physical space (example: 1 MiB to 2 MiB)
    for (uint32_t addr = 0x00100000; addr < 0x00200000; addr += FRAME_SIZE) {
        set_frame(addr / FRAME_SIZE);
    }

    // Load page directory into CR3
    asm volatile ("mov %0, %%cr3" :: "r"(page_directory));

    // Enable paging by setting the PG bit in CR0
    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile ("mov %0, %%cr0" :: "r"(cr0));
    printf("Paging initialized\n");
}

void map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags){

    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;
    printf("pd_index: %d\n", pd_index);
    printf("pt_index: %d\n", pt_index);

    uint32_t* page_table;

    if (!(page_directory[pd_index] & PAGE_PRESENT)) {
        uint32_t frame = first_free_frame();
        if (frame == (uint32_t)-1) {
            printf("map_page: No free frames for page table\n");
            return;
        }
        printf("Frame number: %d\n",frame);
        set_frame(frame);
        uint32_t pt_phys = frame * FRAME_SIZE;

        void* pt_virt = (void*)pt_phys;
        memset(pt_virt, 0, FRAME_SIZE);

        uint32_t pd_flags = PAGE_PRESENT | PAGE_RW;
        if (flags & PAGE_USER) pd_flags |= PAGE_USER;
        page_directory[pd_index] = pt_phys | pd_flags;

        //page_table = (uint32_t*)pt_virt;

        if(virtual_addr>=0xC0000000){
            page_table = (uint32_t*)PHYS_TO_VIRT(pt_phys);
        }else{
            page_table = (uint32_t*)pt_virt;
        }

    }else{
        uint32_t pt_phys = page_directory[pd_index] & ~0xFFF;
        void* pt_virt = (void*)pt_phys;
        //page_table = (uint32_t*)pt_virt;

        if(virtual_addr>=0xC0000000){
            page_table = (uint32_t*)PHYS_TO_VIRT(pt_phys);
        }else{
            page_table = (uint32_t*)pt_virt;
        }
        
    }

    page_table[pt_index] = physical_addr | PAGE_PRESENT | PAGE_RW | (flags & PAGE_USER);
    asm volatile("invlpg (%0)" :: "r" (virtual_addr) : "memory");
    printf("Mapped %x to %x on PDE : %d , PTE : %d\n", physical_addr, virtual_addr, pd_index, pt_index);
}