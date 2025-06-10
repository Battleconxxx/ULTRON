#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/gdt.h>

void syscall_handler() {
    terminal_writestring("syscall received!\n");
}


__attribute__((noreturn)) void user_entry() {
    // printf("Hello from user mode!\n");
    // for(;;){}
    while (1) { asm volatile("hlt"); }

}


void jump_to_user_mode(uint32_t user_stack, uint32_t entry_point) {
    asm volatile (
        "cli\n"                    // disable interrupts
        "mov $0x23, %%ax\n"        // 0x23 = User data selector | RPL 3
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"

        "pushl $0x23\n"            // SS = user data segment
        "pushl %0\n"               // ESP = user stack pointer
        "pushf\n"                  // EFLAGS
        "pushl $0x1B\n"            // CS = user code segment
        "pushl %1\n"               // EIP = user mode entry point
        "iret\n"
        :
        : "r"(user_stack), "r"(entry_point)
		:"memory"
    );
}



void kernel_main(void) {
	terminal_initialize();
	printf("Hello, kernel World!\n");
	printf("int: %d , char: %c , String: %s , hex: %x\n", 1234, 'U', "Batman", 1234);
	printf("Line \nBreak\n");
	gdt_install();
	printf("GDT Setup done");
	
	uint32_t user_stack[1024];
	printf("Stack created");
	uint32_t stack_top = (uint32_t)&user_stack[1024];
	printf("stack top: %x", (uint32_t)stack_top);
	printf("user_entry: %x\n", (uint32_t)user_entry);

	jump_to_user_mode(stack_top, (uint32_t)user_entry);
	printf("END");
}
