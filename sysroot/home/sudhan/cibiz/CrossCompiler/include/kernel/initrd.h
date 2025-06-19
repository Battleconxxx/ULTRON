#ifndef INITRD_H
#define INITRD_H

#include <stddef.h>

void initrd_setup(const void* addr);
void* initrd_find_file(const char* filename, size_t* out_size);


#endif