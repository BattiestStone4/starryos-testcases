#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <linux/futex.h>
#include <sys/syscall.h>

#define NUM_THREADS 10

int futex_value = 0;

typedef struct {
    int id;
    unsigned int wakeup_bit;
} thread_info_t;

void *thread_function(void *arg) {
    thread_info_t *thread_info = (thread_info_t *)arg;
    int rc;

    printf("Thread %d is waiting on futex\n", thread_info->id);
    rc = syscall(SYS_futex, &futex_value, FUTEX_WAIT, 0, NULL, NULL, thread_info->wakeup_bit);
    if (rc == -1) {
        perror("futex wait");
    }

    printf("Thread %d woken up\n", thread_info->id);
    int num_woken_threads = syscall(SYS_futex, &futex_value, FUTEX_WAKE, -1, NULL, NULL, FUTEX_BITSET_MATCH_ANY);
    printf("Woke up %d threads", num_woken_threads);

    printf("Thread %d has been woken up\n", thread_info->id);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    thread_info_t thread_info[NUM_THREADS];
    int rc, i;

    // Create threads and set their wakeup bits
    for (i = 0; i < NUM_THREADS; i++) {
        thread_info[i].id = i;
        thread_info[i].wakeup_bit = 1 << i;

        rc = pthread_create(&threads[i], NULL, thread_function, &thread_info[i]);
        if (rc) {
            fprintf(stderr, "Error creating thread %d: %d\n", i, rc);
            return 1;
        }
    }

    sched_yield();

    // Wake up a subset of the threads using FUTEX_WAKE_BITSET
    unsigned int wakeup_bitset = 0b10101010;
    printf("parent is going to wake up child\n");
    int num_woken_threads = syscall(SYS_futex, &futex_value, FUTEX_WAKE, 1, NULL, NULL, NULL);
    printf("Woke up %d threads", num_woken_threads);

    // Wait for all threads to finish
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}