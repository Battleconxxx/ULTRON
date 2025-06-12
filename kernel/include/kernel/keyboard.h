// #ifndef KEYBOARD_H
// #define  KEYBOARD_H

// #include <stdbool.h>

// #define KEYBOARD_BUFFER_SIZE 128

// bool buffer_empty();
// bool buffer_full();
// void buffer_push(char c);
// char buffer_pop();
// char keyboard_getchar();
// int terminal_readline(char* buf, int maxlen);
// void keyboard_handler(registers_t *regs);

// #endif

#ifndef KEYBOARD_H
#define KEYBOARD_H

void keyboard_init();
char keyboard_getchar();  // blocking
void keyboard_handler();  // IRQ1 handler
int terminal_readline(char* buf, int maxlen);

#endif
