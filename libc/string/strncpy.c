#include <stddef.h>

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;

    // Copy characters from src to dest
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }

    // Pad the rest with null bytes if src is shorter than n
    for (; i < n; i++) {
        dest[i] = '\0';
    }

    return dest;
}