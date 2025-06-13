#ifndef KEYBOARD_BUFFER_H
#define KEYBOARD_BUFFER_H

#pragma once

#define KB_BUFFER_SIZE 256

extern char kb_buffer[KB_BUFFER_SIZE];
extern int kb_head;
extern int kb_tail;

extern volatile int kb_ready;

#endif