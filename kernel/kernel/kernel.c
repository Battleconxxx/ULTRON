#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/interrupts.h>
#include <kernel/user.h>
#include <kernel/memory.h>
#include <kernel/fs.h>

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void jump_to_user_mode(uint32_t user_stack, uint32_t entry_point) {


    uint32_t eflags;
    asm volatile("pushf; pop %0" : "=r"(eflags));
    eflags |= (1 << 9);  // Set IF (interrupt enable)


    asm volatile (
        "cli\n"                    // disable interrupts
        "mov $0x23, %%ax\n"        // 0x23 = User data selector | RPL 3
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"

        "pushl $0x23\n"            // SS = user data segment
        "pushl %0\n"               // ESP = user stack pointer
        "pushl %2\n"                  // EFLAGS
        "pushl $0x1B\n"            // CS = user code segment
        "pushl %1\n"               // EIP = user mode entry point
        "iret\n"
        :
        : "r"(user_stack), "r"(entry_point), "r"(eflags)
		:"memory"
    );
}



void kernel_main(uint32_t magic , multiboot_info_t* mbi) {


	terminal_initialize();
	printf("Hello, kernel World!\n");
	printf("int: %d , char: %c , String: %s , hex: %x\n", 1234, 'U', "Batman", 1234);
	printf("Line \nBreak\n");
	gdt_install();
	printf("GDT Setup done\n");
    init_interrupts();
    printf("Interrupts Initialized\n");
	
	// uint32_t user_stack[2048];
	// printf("User Stack created\n");
	// uint32_t stack_top = (uint32_t)&user_stack[2048];
	// printf("User stack top: %x ", (uint32_t)stack_top);

    uint32_t* user_stack = (uint32_t*)0x200000;
    uint32_t stack_top = (uint32_t)(user_stack + 2048);


	printf("user_entry: %x\n", (uint32_t)user_entry);
    extern uint8_t kernel_stack_top;
    printf("kernel_stack_top: %x\n", (uint32_t)&kernel_stack_top);

    //memory
    parse_memory_map(mbi);
    init_memory();
    //init_paging();

    //RAMFS
    ramfs_init();
    printf("RAMFS initialized\n");

    outb(0x21, 0xFD); // This makes sure all PIC is disabled and only keyboard is enabled
	jump_to_user_mode(stack_top, (uint32_t)user_entry);
	printf("END");
}
