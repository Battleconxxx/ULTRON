#include <stdint.h>
#include <string.h>

#define SYSCALL_WRITE 2
#define SYSCALL_GETCHAR 3

void sys_write(const char* message){
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
}

char sys_getchar() {
    char c;
    asm volatile(
        "movl %1, %%eax\n"    // syscall number
        "int $0x80\n"         // software interrupt
        "movb %%al, %0\n"     // return value in AL (8-bit)
        : "=r"(c)
        : "r"((uint32_t)SYSCALL_GETCHAR)
        : "eax"
    );
    return c;
}

__attribute__((noreturn)) void user_entry() {
    const char *message = "Hello from user mode!\n\0";
    uint32_t ret;
    sys_write(message);    

    message = "Syscall 2 working\n\0";

    sys_write(message);

    char buffer[256];
    while(1){
        sys_write(">");
        int index = 0;
        while (1)
        {
            
        }
        
    }
}