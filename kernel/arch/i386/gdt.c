#include <string.h>
#include <kernel/gdt.h>

struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0;    // Stack pointer to load when switching to ring 0
    uint32_t ss0;     // Stack segment to load when switching to ring 0
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));


static struct gdt_entry gdt[6];

static struct gdt_ptr gp;

static struct tss_entry tss;

extern void gdt_flush(uint32_t);

extern void tss_flush();

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = base & 0xFFFF;
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = limit & 0xFFFF;
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access = access;
}

extern uint8_t kernel_stack_top;

void gdt_install(void){
    gp.limit = sizeof(struct gdt_entry) * 6 - 1 ;
    gp.base = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User code (ring 3, exec/read)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User data (ring 3, read/write)
    write_tss(5, 0x10, (uint32_t)&kernel_stack_top);

    gdt_flush((uint32_t)&gp);
    tss_flush();
}

void write_tss(int num, uint16_t ss0, uint32_t esp0){

    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss) - 1;

    //gdt_set_gate(num, base, limit, 0x89, 0x40); // Present, Ring 0, type 0x9 = 32-bit TSS (available)
    gdt_set_gate(num, base, limit, 0x89, 0x40);

    memset(&tss, 0, sizeof(tss));
    tss.ss0 = ss0;
    tss.esp0 = esp0;
    // tss.cs = 0x1B; // user mode code segment (GDT index 3 | 0x3)
    // tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x23; // user data segment (GDT index 4 | 0x3)
    tss.iomap_base = sizeof(tss);
}