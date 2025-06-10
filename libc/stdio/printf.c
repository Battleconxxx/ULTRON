#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int int_to_string(int num, char *buffer){
	char temp[12];
	int i=0, is_negative=0;

	if(num == 0){
		buffer[0] = '0';
		buffer[1] = '\0';
		return 1;
	}

	if(num < 0){
		is_negative = 1;
		num=-num;
	}

	while(num && i<11){
		temp[i++] = (num%10) + '0';
		num/=10;
	}	

	if(is_negative){
		temp[i++] = '-';
	}

	for(int j=0; j<i; j++){
		buffer[j] = temp[i-j-1];
	}

	buffer[i] = '\0';
	return i;
}


int int_to_hex(unsigned int num, char* buffer){

	const char *hex_digits = "0123456789abcdef";
	char temp[9];

	int i=0;

	if(num == 0){
		buffer[0] = '0';
		buffer[1] = 'x';
		buffer[2] = '0';
		buffer[3] = '\0';
		return 3;
	}

	while(num && i<8){
		temp[i++] = hex_digits[num % 16];
		num >>=4;
	}

	buffer[0] = '0';
	buffer[1] = 'x';

	for(int j=0;j<i;j++){
		buffer[2+j] = temp[i-j-1];
	}

	buffer[2+i] = '\0';
	return 2+i;
}


static bool print(const char* data, size_t length) {
	const unsigned char* bytes = (const unsigned char*) data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return false;
	return true;
}

int printf(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int written = 0;

	while (*format != '\0') {
		size_t maxrem = INT_MAX - written;


		if (*format == '\\') {
		format++;
		char out;
		switch (*format) {
			case 'n': out = '\n'; break;
			case 't': out = '\t'; break;
			case 'r': out = '\r'; break;
			case '\\': out = '\\'; break;
			case '\"': out = '\"'; break;
			case '\'': out = '\''; break;
			default:
				// Unknown escape: print both
				if (!print("\\", 1) || !print(format, 1)) return -1;
				written += 2;
				format++;
				continue;
		}
		if (!print(&out, 1)) return -1;
		written++;
		format++;
		continue;
	}


		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(format, amount))
				return -1;
			format += amount;
			written += amount;
			continue;
		}

		const char* format_begun_at = format++;

		if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int /* char promotes to int */);
			if (!maxrem) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(&c, sizeof(c)))
				return -1;
			written++;
		} else if (*format == 's') {
			format++;
			const char* str = va_arg(parameters, const char*);
			size_t len = strlen(str);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(str, len))
				return -1;
			written += len;
		} else if (*format == 'd') {
			format++;
			const int num = va_arg(parameters, int);
			char buffer[12];
			size_t len = int_to_string(num, buffer);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(buffer, len))
				return -1;
			written += len;
		} else if (*format == 'x') {
			format++;
			const unsigned int num = va_arg(parameters,unsigned int);
			char buffer[9];
			size_t len = int_to_hex(num, buffer);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(buffer, len))
				return -1;
			written += len;
		}else {
			format = format_begun_at;
			size_t len = strlen(format);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(format, len))
				return -1;
			written += len;
			format += len;
		}
	}

	va_end(parameters);
	return written;
}
