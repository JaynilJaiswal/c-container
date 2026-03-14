#include <stdio.h>
#include <unistd.h>

#define FLOAT_CHUNK 2048

int main() {
    float buf[FLOAT_CHUNK];
    ssize_t bytes_read;
    double dot_product = 0.0;
    size_t total_floats = 0;

    while ((bytes_read = read(0, buf, sizeof(buf))) > 0) {
        size_t count = bytes_read / sizeof(float);
        for (size_t i = 0; i < count; i++) {
            dot_product += buf[i] * 0.999f; // Simulated tensor transformation
        }
        total_floats += count;
    }

    printf("--> [Matrix Worker] Processed %.2f MB (%zu floats) | Dot Product: %.2f\n", 
            (total_floats * sizeof(float)) / 1048576.0, total_floats, dot_product);
    return 0;
}