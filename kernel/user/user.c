#include <stdint.h>

#define SYSCALL_WRITE 2

__attribute__((noreturn)) void user_entry() {
    const char *message = "Hello from user mode!\n\0";

    // Call SYSCALL_WRITE with the string pointer
    uint32_t ret;
    asm volatile(
        "movl %1, %%eax\n"    // Syscall number (SYSCALL_WRITE) in eax
        "movl %2, %%ebx\n"    // String pointer in ebx
        "int $0x80\n"         // Trigger syscall
        "movl %%eax, %0"      // Store return value from eax
        : "=r"(ret)           // Output: return value
        : "r"((uint32_t)SYSCALL_WRITE), "r"((uint32_t)message) // Inputs
        : "eax", "ebx"        // Clobbered registers
    );

    message = "Syscall 2 working\n\0";

    // Call SYSCALL_WRITE with the string pointer
    asm volatile(
        "movl %1, %%eax\n"    // Syscall number (SYSCALL_WRITE) in eax
        "movl %2, %%ebx\n"    // String pointer in ebx
        "int $0x80\n"         // Trigger syscall
        "movl %%eax, %0"      // Store return value from eax
        : "=r"(ret)           // Output: return value
        : "r"((uint32_t)SYSCALL_WRITE), "r"((uint32_t)message) // Inputs
        : "eax", "ebx"        // Clobbered registers
    );
    for(;;){}
}