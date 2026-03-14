#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define CHUNK 8192

int main() {
    unsigned char buf[CHUNK];
    ssize_t n;
    uint64_t hash = 0xcbf29ce484222325;
    size_t total = 0;

    while ((n = read(0, buf, CHUNK)) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            hash ^= buf[i];
            hash *= 0x00000100000001B3;
        }
        total += n;
    }

    printf("--> [Hash Worker] Processed %.2f MB | FNV-1a: 0x%llx\n", 
            total / 1048576.0, (unsigned long long)hash);
    return 0;
}