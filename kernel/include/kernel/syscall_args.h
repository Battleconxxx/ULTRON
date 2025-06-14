#ifndef SYSCALL_ARGS_H
#define SYSCALL_ARGS_H

#include <stdint.h>

typedef struct {
    uint32_t fd;
    const void* buf;
    uint32_t count;
} syscall_args_t;

// Declare a global instance
extern syscall_args_t g_syscall_args;

#endif
