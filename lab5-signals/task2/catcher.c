#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>




// global variables to store current mode value and amount of mode requests
// we use volatile to avoid optimizing, that system used because the handler is called by system, not by our program
volatile int mode = -1;
volatile int mode_requests = 0;

// handler is used to update mode due to get argument from sender and increase amount of requests
void updateModeHandler(int mode_get){
    mode = mode_get;
    mode_requests ++;
}

// function for alternative signal processing, it uses signal info structure
void SIGUSR1_action(int signo, siginfo_t *info, void *extra) {
    int pid = info->si_pid;
    int mode_get = info->si_int;
    updateModeHandler(mode_get);

    printf("Catcher received mode: %d from process: %d\n", mode_get, pid);

    // send signal to the process with got PID (sender)
    kill(pid, SIGUSR1);
}

// function to set sigaction
void set_sigaction(){

    // register struct to use it later in parameters of sigaction()
    struct sigaction act;
    act.sa_sigaction = SIGUSR1_action;

    // by using this flag, struct siginfo_t will be sent to the function
    act.sa_flags = SA_SIGINFO;

    // clear list of signals that should be masked
    sigemptyset(&act.sa_mask);

    // register signal processing
    sigaction(SIGUSR1, &act, NULL);
}

int main(){
    printf("Catcher PID: %d\n", getpid());

    set_sigaction();

    while (1){
        switch(mode){
            case 1: {
                for (int i = 1; i <= 100; i ++) printf("%i\n", i);
                printf("\n");
                mode = -1;
                break;
            }
            case 2: {
                printf("Mode requests amount: %d\n", mode_requests);
                mode = -1;
                break;
            }
            case 3: {
                printf("Exiting...\n");
                exit(0);
            }
            default:
                break;
        }
    }
}