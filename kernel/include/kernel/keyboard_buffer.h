#ifndef KEYBOARD_BUFFER_H
#define KEYBOARD_BUFFER_H

#define KB_BUFFER_SIZE 256

char kb_buffer[KB_BUFFER_SIZE];
int kb_head = 0;
int kb_tail = 0;

#endif