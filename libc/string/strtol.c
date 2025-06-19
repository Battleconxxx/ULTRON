long strtol(const char* nptr, char** endptr, int base) {
    const char* s = nptr;
    long result = 0;

    // Skip whitespace
    while (*s == ' ' || *s == '\t' || *s == '\n')
        s++;

    // Optional sign
    int negative = 0;
    if (*s == '-') {
        negative = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    // Parse digits
    while (1) {
        char c = *s;
        int digit;

        if ('0' <= c && c <= '9') digit = c - '0';
        else if ('a' <= c && c <= 'f') digit = c - 'a' + 10;
        else if ('A' <= c && c <= 'F') digit = c - 'A' + 10;
        else break;

        if (digit >= base) break;

        result = result * base + digit;
        s++;
    }

    if (endptr) *endptr = (char*)s;
    return negative ? -result : result;
}