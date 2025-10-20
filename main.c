#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define STACK_SIZE (1024*1024)

int container_initialize(void *args) {
    printf("Welcome to the container \n");

    char* new_hostname = "myhost";
    sethostname(new_hostname, strlen(new_hostname));

    char *const argv[] = {"/bin/sh", NULL};
    execv(argv[0], argv);

    printf("Oops, something went wrong with execv \n");
    return 1;
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./c-container run <command>\n"); //
        return 1;
    }

    if (strcmp(argv[1], "run") == 0) {
        printf("Allocating memory ...\n");
        char *buffer = malloc(STACK_SIZE);
        if (!buffer) {
            perror("malloc");
            return 1;
        }
        char *buffer_end = buffer + STACK_SIZE;
        printf("Calling clone() ...\n");
        int flags = CLONE_NEWUTS | SIGCHLD;
        pid_t pid = clone(container_initialize, buffer_end, flags, NULL);
        if (pid <0) {
            perror("clone");
            free(buffer);
            return 1;
        }

        waitpid(pid, NULL, 0);

        printf("%d Container exited ...\n", pid);
        free(buffer);

    } else {
        fprintf(stderr, "Uknown command: %s\n", argv[1]);
        return 1;
    }
    return 0;
}


