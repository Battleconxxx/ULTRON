#include <kernel/tty.h>
#include <kernel/keyboard_buffer.h>
#include <kernel/interrupts.h>

// Define here
char kb_buffer[KB_BUFFER_SIZE];
int kb_head = 0;
int kb_tail = 0;
volatile int kb_ready = 0;


// #define KEYBOARD_BUFFER_SIZE 128

// static char buffer[KEYBOARD_BUFFER_SIZE];
// static int buffer_head = 0;
// static int buffer_tail = 0;



// static inline int buffer_empty() {
//     return buffer_head == buffer_tail;
// }

// static inline int buffer_full() {
//     return ((buffer_head + 1) % KEYBOARD_BUFFER_SIZE) == buffer_tail;
// }

// static void buffer_push(char c) {
//     if (!buffer_full()) {
//         buffer[buffer_head] = c;
//         buffer_head = (buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
//     }
// }

// static char buffer_pop() {
//     if (!buffer_empty()) {
//         char c = buffer[buffer_tail];
//         buffer_tail = (buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
//         return c;
//     }
//     return 0;
// }

// const char scancode_to_ascii[128] = {
//     0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
//     '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
//     'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\',
//     'z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' ',
// };

// void keyboard_handler() {
//     unsigned char scancode = inb(0x60);

//     if (!(scancode & 0x80)) {  // Ignore key releases
//         char c = scancode_to_ascii[scancode];
//         if (c) buffer_push(c);
//     }

//     outb(0x20, 0x20);  // Send EOI to PIC
// }



// char keyboard_getchar() {
//     while (buffer_empty()) {
//         asm volatile("hlt");  // sleep until next interrupt
//     }
//     return buffer_pop();
// }

// int terminal_readline(char* buf, int maxlen) {
//     int i = 0;
//     while (i < maxlen - 1) {
//         char c = keyboard_getchar();  // blocking keyboard input
//         if (c == '\n') break;

//         buf[i++] = c;
//         terminal_putchar(c);          // echo to screen
//     }
//     buf[i] = '\0';
//     return i;
// }

// void keyboard_init() {
//     // Unmask IRQ1 (keyboard)
//     uint8_t mask = inb(0x21);
//     mask &= ~(1 << 1);  // clear bit 1 to enable IRQ1
//     outb(0x21, mask);
// }
