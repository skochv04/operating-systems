#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>

#include "print_settings.h"

// update the variable in signal handler
volatile bool close_flag = false;

void sig_handler(int signum) {
    close_flag = true;
}

void generate_string(char* buffer, int user_number) {
    // use user_number to generate unique string
    unsigned int seed = time(NULL) + user_number;

    for (int i = 0; i < USER_BUFFER; i++) {
        buffer[i] = 'a' + rand_r(&seed) % 26;
    }
    buffer[USER_BUFFER] = '\0'; // endline
}


int main(int argc, char** argv){
    // we need 2 argument to start: the source file and amount of users
    if (argc < 2) {
        printf("Can not proceed, not enough arguments\n");
        return 1;
    }

    // convert amount of users to long value
    long users_amount = strtol(argv[1], NULL, 10);

    // register handler closing client to all signals
    for (int sig = 1; sig < SIGRTMAX; sig++) {
        signal(sig, sig_handler);
    }

    // open shared memory
    int memory_fd = shm_open(SHARED_MEMORY, O_RDWR, S_IRUSR | S_IWUSR);
    if (memory_fd == -1) perror("shared memory open");

    // attach memory to process
    memory* memory_map = mmap(NULL, sizeof (memory), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, memory_fd, 0);
    if (memory_map == MAP_FAILED) perror("shared memory mmap");

    // buffer for users` messages
    char buffer[USER_BUFFER + 1];

    // spawn given amount of users
    for (int i = 0; i < users_amount; i ++){
        pid_t pid = fork();
        if (pid == -1) perror("fork");
        else if (pid == 0){
            // child process
            while(!close_flag){
                generate_string(buffer, i);
                int notBusyPrinter = -1;
                int sem_val;

                // find not busy printer if it is possible
                for (int j = 0; j < memory_map->printers_amount; j++){
                    sem_getvalue(&memory_map->printers[j].printer_semaphore, &sem_val);
                    if (sem_val > 0){
                        notBusyPrinter = j;
                        break;
                    }
                }

                // if we didn`t find not busy printer, find random printer to wait for it will not be busy
                if (notBusyPrinter == -1) notBusyPrinter = rand() % memory_map->printers_amount;

                // decrement semaphore (wait if printer is busy)
                if (sem_wait(&memory_map->printers[notBusyPrinter].printer_semaphore) < 0) perror("semaphore wait");

                // copy data from buffer to printer buffer and set printing status
                memcpy(memory_map->printers[notBusyPrinter].printer_buffer, buffer, MAX_PRINTER_BUFFER);
                memory_map->printers[notBusyPrinter].isPrinting = true;
                memory_map->printers[notBusyPrinter].printer_buffer_size = strlen(buffer);

                printf("Successful send data from user %d to printer %d\n", i, notBusyPrinter);
                fflush(stdout);

                // sleep for random (from 0..5) amount of time before continuing
                sleep(rand() % 5 + 1);
            }
            exit(0);
        }
    }

    // wait until the end of all child processes (sending data to print)
    while(wait(NULL) > 0);

    // unmap memory
    munmap(memory_map, sizeof(memory));
}