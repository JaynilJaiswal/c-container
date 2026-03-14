#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: ./data_generator <type> <size_mb>\n");
        return 1;
    }
    
    char *type = argv[1];
    size_t size_mb = atoi(argv[2]);
    size_t bytes = size_mb * 1024 * 1024;
    char *buffer = malloc(bytes);

    if (strcmp(type, "log") == 0) {
        const char *log200 = "127.0.0.1 - - [14/Mar/2026] \"GET /api/data HTTP/1.1\" 200 1024\n";
        const char *log500 = "127.0.0.1 - - [14/Mar/2026] \"POST /api/auth HTTP/1.1\" 500 256\n";
        for (size_t i = 0; i < bytes;) {
            const char *line = (rand() % 100 < 5) ? log500 : log200;
            size_t len = strlen(line);
            if (i + len > bytes) break;
            memcpy(buffer + i, line, len);
            i += len;
        }
    } else if (strcmp(type, "matrix") == 0) {
        float *floats = (float*)buffer;
        size_t count = bytes / sizeof(float);
        for (size_t i = 0; i < count; i++) floats[i] = 1.0001f;
    } else {
        memset(buffer, 0xAB, bytes); 
    }

    // Blast the payload to standard output
    write(STDOUT_FILENO, buffer, bytes);
    free(buffer);
    return 0;
}