#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include "print_settings.h"

// update the variable in signal handler
volatile bool close_flag = false;

void sig_handler(int signum) {
    close_flag = true;
}

int main(int argc, char** argv) {
    // we need 2 argument to start: the source file and amount of printers
    if (argc < 2) {
        printf("Can not proceed, not enough arguments\n");
        return 1;
    }

    // convert amount of printers to long value
    int printers_amount = strtol(argv[1], NULL, 10);

    // check if printers_amount is not bigger than max_amount
    if (printers_amount > MAX_PRINTERS){
        printf("Can not proceed, given amount of printers is greater than maximum printers amount");
        return -1;
    }

    // register handler closing client to all signals
    for (int sig = 1; sig < SIGRTMAX; sig++) {
        signal(sig, sig_handler);
    }

    // open shared memory
    // system use O_CREAT flag to create shared memory, users will also use it
    int memory_fd = shm_open(SHARED_MEMORY, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (memory_fd == -1) perror("shared memory open");

    // determine size of memory
    if (ftruncate(memory_fd, sizeof (memory)) == -1) perror("ftruncate");

    // attach memory to process
    memory* memory_map = mmap(NULL, sizeof (memory), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, memory_fd, 0);
    if (memory_map == MAP_FAILED) perror("shared memory mmap");

    // clean shared memory
    memset(memory_map, 0, sizeof (memory));

    // set number of printers
    memory_map->printers_amount = printers_amount;

    // spawn given amount of printers
    for (int i = 0; i < printers_amount; i++){
        // int sem_init(sem_t *sem, int pshared, unsigned int value);
        // pshared = 0/1 means, if the semaphore will be shared or not
        sem_init(&memory_map->printers[i].printer_semaphore, 1, 1);

        pid_t pid = fork();
        if (pid == -1) perror("fork");
        else if (pid == 0){
            // child process
            while(!close_flag){
                // if user set printing status to True, then print
                if (memory_map->printers[i].isPrinting){
                    // print data that user send, but only 1 character with 1s delay
                    for (int j = 0; j < memory_map->printers[i].printer_buffer_size; j++){
                        printf("%c", memory_map->printers[i].printer_buffer[j]);
                        sleep(1);
                    }

                    printf("\n");
                    fflush(stdout);

                    memory_map->printers[i].isPrinting = false;

                    // increment semaphore to signalize that printer is not busy
                    sem_post(&memory_map->printers[i].printer_semaphore);
                }
            }
            exit(0);
        }
    }

    // wait until the end of all child processes (printing data)
    while(wait(NULL) > 0);

    // destroy all semaphores
    for (int i = 0; i < printers_amount; i++){
        sem_destroy(&memory_map->printers[i].printer_semaphore);
    }

    // unmap memory
    munmap(memory_map, sizeof(memory));

    // mark memory to delete using unlink
    shm_unlink(SHARED_MEMORY);

    return 0;
}