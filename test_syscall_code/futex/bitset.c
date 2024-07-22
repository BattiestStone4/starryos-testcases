#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <pthread.h>
#include <errno.h>

#define NUM_THREADS 4

int futex_value = 0;
pthread_t threads[NUM_THREADS];

void *thread_func(void *arg) {
    int thread_id = *(int *)arg;
    int bitset = 1 << thread_id;
    int ret;

    printf("Thread %d waiting on futex...\n", thread_id);
    ret = syscall(SYS_futex, &futex_value, FUTEX_WAIT_BITSET, 0, NULL, NULL, bitset);
    if (ret < 0) {
        if (errno == EAGAIN) {
            printf("Thread %d did not need to wait, continuing...\n", thread_id);
        } else {
            perror("FUTEX_WAIT_BITSET");
            return NULL;
        }
    }

    printf("Thread %d woke up!\n", thread_id);
    ret = syscall(SYS_futex, &futex_value, FUTEX_WAKE_BITSET, 1, NULL, NULL, 0b0101);
    return NULL;
}

int main() {
    int i, thread_ids[NUM_THREADS];

    // Create the threads
    for (i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }

    // Wait for a while to let the threads block on the futex
    sleep(1);

    // Wake up the threads with FUTEX_WAKE_BITSET
    printf("Waking up threads...\n");
    int ret = syscall(SYS_futex, &futex_value, FUTEX_WAKE_BITSET, 2, NULL, NULL, 0b1010);
    printf("Woke up %d threads.\n", ret);

    // Wait for the threads to finish
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}