#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    // we need 2 arguments to start: the source file, input and output directories
    if (argc < 2) {
        printf("Can not proceed, not enough arguments\n");
        return 1;
    }

    // convert argument to long to deal with it on the next steps
    long num = strtol(argv[1], NULL, 10);

    if (num < 0){
        printf("Can not proceed, number of processes should not be less than 0\n");
        return 1;
    }

    for (int i = 0; i < num; i++){
        pid_t child_pid = fork();
        if(child_pid==0) {
            // child's part of code
            printf("Child process: Child pid:%d\n",(int)getpid());
            printf("Child process. Parent pid:%d\n",(int)getppid());

            // success, exit
            exit(0);
        }
    }

    // parent waits until all children processes will be ended
    while(wait(NULL) > 0);

    // parent's part of code
    printf("%ld\n", num);

    return 0;
}