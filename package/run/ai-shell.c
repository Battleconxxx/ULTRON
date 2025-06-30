#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>

void strip_ansi(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (*src == '\033') {  // ESC
            src++;
            if (*src == '[') {
                src++;
                while ((*src >= '0' && *src <= '9') || *src == ';') src++;
                if (*src == 'm') src++;
            }
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

int main() {
    char input[1024];
    FILE *fp;

    while (1) {
        printf("🧠 Ask: ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) break;

        // Write input to a temp file
        FILE *tmp = fopen("/tmp/prompt.txt", "w");
        fprintf(tmp, "%s", input);
        fclose(tmp);

        // Run the model and capture output
        // fp = popen("/usr/share/run/llama-run Shell_command.bin < /tmp/prompt.txt", "r");
        fp = popen("/usr/share/run/llama-run /usr/share/run/Shell_command.bin < /tmp/prompt.txt", "r");

        char result[1024];
        if (fgets(result, sizeof(result), fp)) {
            strip_ansi(result);
            printf("🤖 Model: %s\n", result);
            printf("💻 Running: %s\n", result);
            system(result);  // Execute it!
        }

        pclose(fp);
    }

    return 0;
}
