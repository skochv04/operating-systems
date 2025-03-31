#define _XOPEN_SOURCE 700
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

void handler(int signo) {
    printf("Feedback received!\n");
}

int main(int argc, char** argv){
    // we need 2 arguments to start: the source file, PID of catcher and mode of work
    if (argc < 3) {
        printf("Can not proceed, not enough arguments\n");
        return 1;
    }

    // register the confirmation handler
    signal(SIGUSR1, handler);

    // convert got arguments to numbers
    long signal_pid = strtol(argv[1], NULL, 10);
    long signal_argument = strtol(argv[2], NULL, 10);

    if (signal_argument < 1 || signal_argument > 3) {
        printf("Signal argument %ld is incorrect!\n", signal_argument);
        return 0;
    }

    // create sigval_t to use it in sigqueue parameters
    union sigval value = {signal_argument};
    sigqueue(signal_pid, SIGUSR1, value);
    printf("Signal sent, given argument: %ld\n", signal_argument);

    // create a mask as a set of full signals, deleting SIGUSR1 and SIGINT from there, so that signals will not be ignored
    sigset_t mask;
    sigfillset(&mask);

    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGINT);

    // wait for feedback, replaces the process mask of signals with the parameter mask and waits
    sigsuspend(&mask);
    return 0;
}