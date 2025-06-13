#include <stdint.h>
#include <string.h>
#include <kernel/keyboard_buffer.h>

#define SYSCALL_WRITE 2
#define SYSCALL_GETCHAR 3
#define SYSCALL_CLEAR 4

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
    while (kb_head == kb_tail);  // Wait for available char

    char c = kb_buffer[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUFFER_SIZE;
    return c;
}

void sys_clear(){
    uint32_t ret;
    asm volatile(
        "movl %1, %%eax\n"    // Syscall number (SYSCALL_WRITE) in eax
        "int $0x80\n"         // Trigger syscall
        "movl %%eax, %0"      // Store return value from eax
        : "=r"(ret)           // Output: return value
        : "r"((uint32_t)SYSCALL_CLEAR)
        : "eax"        // Clobbered registers
    );
}

__attribute__((noreturn)) void user_entry() {
    const char *message = "Hello from user mode!\n\0";
    uint32_t ret;
    sys_write(message);    

    message = "Syscall 2 working\n\0";

    sys_write(message);

    char input[256];
    while(1){
        sys_write(">");
        int index = 0;
        while (1)
        {
            char c = sys_getchar();

            // Echo back
            char echo[2] = {c, '\0'};
            sys_write(echo);

            if (c == '\n') {
                input[index] = '\0';
                break;
            } else if (c == '\b' && index > 0) {
                index--;
            } else if (index < 255) {
                input[index++] = c;
            }
        }
        
        // Now input[] holds the complete command
        if (strncmp(input, "hello", 5) == 0) {
            sys_write("Hi there!\n");
        } else if (strncmp(input, "clear", 5) == 0) {
            sys_write("\033[2J\033[H\n");
            //sys_clear();
        } else if (strncmp(input, "exit", 4) == 0) {
            sys_write("Exiting...\n");
            while (1);
        } else {
            sys_write("Unknown command\n");
        }
    }
}