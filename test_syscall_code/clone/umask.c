#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/wait.h>

#define STACK_SIZE (1024 * 1024)

void print_umask(const char *label) {
    mode_t current_umask = umask(0);
    umask(current_umask);
    printf("%s: %04o\n", label, current_umask);
}

int child_function(void *arg) {
    print_umask("Child init umask"); 
    umask(027);
    print_umask("Child current umask");  
    return EXIT_SUCCESS;
}

int main() {
    int status;
    void *stack = malloc(STACK_SIZE);
    if (stack == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    print_umask("Parent init umask");
    pid_t pid = clone(child_function, (char *) stack + STACK_SIZE,
                    CLONE_VM | SIGCHLD|CLONE_FS | SIGCHLD, NULL);
    if (pid == -1) {
        perror("clone() error");
        return EXIT_FAILURE;
    }
    
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid() error");
        return EXIT_FAILURE;
    }

    print_umask("Parent current umask");
    if (WIFEXITED(status)) {
        printf("Child process exited with status %d\n", WEXITSTATUS(status));
    } else {
        printf("Child process did not exit normally\n");
    }
    
    free(stack); 
    return EXIT_SUCCESS;
}

