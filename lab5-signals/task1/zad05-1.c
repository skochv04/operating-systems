#define _POSIX_SOURCE 200809L
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

void handler(int signo){
    printf("Attention! Signal %d got!\n", signo);
}

int main(int argc, char** argv){
    // we need 1 argument to start: the source file and input string
    if (argc < 2) {
        printf("Can not proceed, not enough arguments\n");
        return 1;
    }

    if (!strcmp(argv[1], "none")){
        signal(SIGUSR1, SIG_DFL);
        raise(SIGUSR1);
    }
    else if (!strcmp(argv[1], "ignore")){
        signal(SIGUSR1, SIG_IGN);
        raise(SIGUSR1);
    }
    else if (!strcmp(argv[1], "handler")){
        signal(SIGUSR1, handler);
        raise(SIGUSR1);
    }
    else if (!strcmp(argv[1], "mask")){
        sigset_t newmask;
        sigemptyset(&newmask);
        sigaddset(&newmask, SIGUSR1);
        sigprocmask(SIG_SETMASK, &newmask, NULL);

        raise(SIGUSR1);

        sigset_t masked;
        sigpending(&masked);
        if (sigismember(&masked, SIGUSR1)) {
            printf("OK, your pending signal is visible\n");
        }
        else printf("NOT OK, your pending signal id not visible\n");
    }

    return 0;
}

