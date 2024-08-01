#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/wait.h>

#define STACK_SIZE (1024 * 1024)

int child_function(void *arg) {
    char *new_dir = "/tmp";
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Child process current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return EXIT_FAILURE;
    }
    
    if (chdir(new_dir) == -1) {
        perror("chdir() error");
        return EXIT_FAILURE;
    }
    
    printf("Child process changed working directory to: %s\n", new_dir);
    
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Child process current working directory after change: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main() {
    int status;
    void *stack = malloc(STACK_SIZE);
    if (stack == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Parent process current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return EXIT_FAILURE;
    }
    
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

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Parent process current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return EXIT_FAILURE;
    }
  
    if (WIFEXITED(status)) {
        printf("Child process exited with status %d\n", WEXITSTATUS(status));
    } else {
        printf("Child process did not exit normally\n");
    }
    
    free(stack);
    return EXIT_SUCCESS;
}

