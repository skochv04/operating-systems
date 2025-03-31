#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// declare global variable
int global = 0;

int main(int argc, char** argv) {
    // we need 2 arguments to start: the source file, input and output directories
    if (argc < 2) {
        printf("Can not proceed, not enough arguments\n");
        return 1;
    }

    // print name of the program
    printf("%s \n", strrchr(argv[0], '/') + 1);

    // declare local variable
    int local = 0;

    // get returned value from fork function
    pid_t child_pid = fork();

    // if fork failed
    if (child_pid == -1) {
        perror("fork");
        return 1;
    }
    else if (child_pid == 0){
        // child's part of code
        printf("Child process\n");

        // increment variables
        global++;
        local++;

        printf("Child pid = %d, Parent pid = %d \n", (int)getpid(), (int)getppid());
        printf("Child's global = %d, Child's local = %d \n", global, local);

        int status = execl("/bin/ls", "ls", "-l", argv[1], NULL);

        // this code will be executed if execl() fails
        exit(status);
    }
    else{
        // child's error code status will be placed in a status variable after wait
        int status = 0;
        wait(&status);

        // get status that child's process returned
        int child_status = WEXITSTATUS(status);

        // parent's part of code
        printf("Parent process\n");
        printf("Parent pid = %d, Child pid = %d\n", (int)getpid(), (int)child_pid);
        printf("Child exit code = %d\n", child_status);
        printf("Parent's global = %d, Parent's local = %d\n", global, local);
        return (child_status);
    }

    return 0;

}