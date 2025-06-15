#include <stdint.h>
#include <string.h>
#include <kernel/keyboard_buffer.h>
#include <kernel/syscall_args.h>
#include <kernel/virtio_serial.h>

#define O_RDWR 0x0002

#define SYSCALL_WRITE 2
#define SYSCALL_GETCHAR 3
#define SYSCALL_CLEAR 4
#define SYSCALL_OPEN 5
#define SYSCALL_READFD 6
#define SYSCALL_WRITEFD 7
#define SYSCALL_CLOSEFD 8

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

int sys_open(const char* path, int flags) {
    int ret;
    asm volatile (
        "movl %1, %%eax\n"
        "movl %2, %%ebx\n"
        "movl %3, %%ecx\n"
        "int $0x80\n"
        "movl %%eax, %0"
        : "=r"(ret)
        : "r"(SYSCALL_OPEN), "r"(path), "r"(flags)
        : "eax", "ebx", "ecx"
    );
    return ret;
}

void set_syscall_args(uint32_t fd, const void* buf, uint32_t count) {
    syscall_args_t* args = (syscall_args_t*) 0xC0000000;  // shared memory location (adjust address)
    args->fd = fd;
    args->buf = buf;
    args->count = count;
}

int sys_writefd(int fd, const void* buf, int count) {
    set_syscall_args(fd, buf, count);
    uint32_t ret;
    asm volatile (
        "movl %1, %%eax\n"
        "int $0x80\n"
        "movl %%eax, %0\n"
        : "=r"(ret)
        : "r"((uint32_t)SYSCALL_WRITEFD)
        : "eax"
    );
    return ret;
}


int sys_readfd(int fd, void* buf, int count) {
    set_syscall_args(fd, buf, count);
    uint32_t ret;
    asm volatile (
        "movl %1, %%eax\n"
        "int $0x80\n"
        "movl %%eax, %0\n"
        : "=r"(ret)
        : "r"((uint32_t)5)  // SYSCALL_READFD
        : "eax"
    );
    return ret;
}

int sys_closefd(int fd) {
    uint32_t ret;
    asm volatile (
        "movl %1, %%eax\n"   // syscall number
        "movl %2, %%ebx\n"   // fd
        "int $0x80\n"
        "movl %%eax, %0\n"
        : "=r"(ret)
        : "r"((uint32_t)SYSCALL_CLOSEFD), "r"((uint32_t)fd)
        : "eax", "ebx"
    );
    return ret;
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
        } else if (strncmp(input, "send", 4) == 0) {
            sys_write("send...\n");
        } else if (strncmp(input, "ai ", 3) == 0) {
            // Send input+3 to the AI bridge via /dev/virtio-ports/ai
            int fd = sys_open("/dev/virtio-ports/ai", O_RDWR);
            if (fd < 0) {
                sys_write("AI device not available\n");
            } else {
                sys_writefd(fd, input + 3, strlen(input + 3));
                sys_writefd(fd, "\n", 1); // Send newline delimiter

                char response[1024];
                int n = sys_readfd(fd, response, sizeof(response) - 1);
                response[n] = '\0';
                sys_write(response);
                virtio_serial_send("hello from OS\n", 14);
                sys_closefd(fd);
            }
        } else {
            sys_write("Unknown command\n");
        }
    }
}