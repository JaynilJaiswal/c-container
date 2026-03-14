#include <stdio.h>
#include <string.h>

int main() {
    char line[512];
    size_t total_requests = 0;
    size_t error_500s = 0;
    size_t total_bytes = 0;

    // Reading from STDIN stream directly
    while (fgets(line, sizeof(line), stdin)) {
        total_requests++;
        total_bytes += strlen(line);
        if (strstr(line, "\" 500 ")) {
            error_500s++;
        }
    }

    printf("--> [Log Worker] Processed %.2f MB | Total Req: %zu | 500 Errors: %zu\n", 
            total_bytes / 1048576.0, total_requests, error_500s);
    return 0;
}