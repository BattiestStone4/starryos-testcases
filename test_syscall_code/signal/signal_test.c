#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handler(int sig) {
    write(STDOUT_FILENO, "Signal received!\n", 17);
}

int main() {
    struct sigaction sa;
    sigset_t newset, oldset;

    // set sig_handler
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    // set empty set
    sigemptyset(&newset);
    sigaddset(&newset, SIGUSR1);

    // block SIGUSR1
    sigprocmask(SIG_BLOCK, &newset, &oldset); 
    printf("SIGUSR1 blocked. Try sending signal now...\n");
    raise(10);
    sleep(1);

    // unblock and wait 
    printf("Unblocking SIGUSR1\n");
    sigprocmask(SIG_UNBLOCK, &newset, NULL);
    kill(getpid(), SIGUSR1);

    return 0;
}
