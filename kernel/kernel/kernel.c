#include <stdio.h>

#include <kernel/tty.h>

void kernel_main(void) {
	terminal_initialize();
	printf("Hello, kernel World!\n");
	printf("int: %d , char: %c , String: %s , hex: %x", 1234, 'U', "Batman", 1234);
}
