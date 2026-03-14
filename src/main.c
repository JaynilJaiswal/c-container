#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

#define STACK_SIZE (1024 * 1024)

struct container_config {
    int channel_fd[2];
    char *worker_path;
};

int container_initialize(void *args) {
    struct container_config *config = (struct container_config*)args;
    
    // Receiver closes write end
    close(config->channel_fd[1]);

    // Map pipe read end to STDIN
    dup2(config->channel_fd[0], STDIN_FILENO);
    close(config->channel_fd[0]);

    // Isolate hostname
    sethostname("nano-worker", 11);

    // Execute the requested worker
    char *const argv[] = {config->worker_path, NULL};
    execv(argv[0], argv);

    perror("execv failed");
    return 1;
}

void generate_and_send_payload(int fd, const char* worker_path, size_t size_mb) {
    size_t bytes = size_mb * 1024 * 1024;
    char *buffer = malloc(bytes);
    
    if (strstr(worker_path, "log_worker")) {
        // Generate simulated Nginx logs with occasional 500 errors
        const char *log200 = "127.0.0.1 - - [14/Mar/2026] \"GET /api/data HTTP/1.1\" 200 1024\n";
        const char *log500 = "127.0.0.1 - - [14/Mar/2026] \"POST /api/auth HTTP/1.1\" 500 256\n";
        for (size_t i = 0; i < bytes;) {
            const char *line = (rand() % 100 < 5) ? log500 : log200; // 5% error rate
            size_t len = strlen(line);
            if (i + len > bytes) break;
            memcpy(buffer + i, line, len);
            i += len;
        }
    } else if (strstr(worker_path, "matrix_worker")) {
        // Generate binary float data (simulated tensor weights)
        float *floats = (float*)buffer;
        size_t count = bytes / sizeof(float);
        for (size_t i = 0; i < count; i++) floats[i] = 1.0001f;
    } else {
        // Generate raw binary data for hashing
        memset(buffer, 0xAB, bytes); 
    }

    write(fd, buffer, bytes);
    free(buffer);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: ./c-container run <worker_path> <payload_mb>\n");
        return 1;
    }

    char *worker_path = argv[2];
    size_t payload_mb = atoi(argv[3]);

    char *stack = malloc(STACK_SIZE);
    struct container_config config;
    config.worker_path = worker_path;

    if (pipe(config.channel_fd) == -1) {
        perror("pipe");
        return 1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int flags = CLONE_NEWUTS | CLONE_NEWPID | SIGCHLD;
    pid_t pid = clone(container_initialize, stack + STACK_SIZE, flags, &config);
    
    if (pid < 0) {
        perror("Clone failed (Did you run with sudo?)");
        free(stack);
        return 1;
    }
    
    // Sender closes read end
    close(config.channel_fd[0]);

    printf("[Host] Pumping %zu MB to %s...\n", payload_mb, worker_path);
    generate_and_send_payload(config.channel_fd[1], worker_path, payload_mb);
    
    // Close write end to send EOF to the worker
    close(config.channel_fd[1]);
    
    waitpid(pid, NULL, 0);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("[Host] Total Lifecycle Time: %.4f sec\n", elapsed);
    free(stack);
    return 0;
}