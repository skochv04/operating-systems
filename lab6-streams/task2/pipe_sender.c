#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#define FIFO_IN_PATH "build/pipe_in"
#define FIFO_OUT_PATH "build/pipe_out"

void unlink_pipes(){
    unlink(FIFO_IN_PATH);
    unlink(FIFO_OUT_PATH);
}

int main(int argc, char** argv){
    // we need 3 argument to start: the source file, left border, right border and amount of ranges
    if (argc < 4) {
        printf("Can not proceed, not enough arguments\n");
        return 1;
    }

    if (mknod(FIFO_IN_PATH, 0666, 0) < 0) {
        perror("Failed to create pipe");
        unlink_pipes();
        exit(EXIT_FAILURE);
    }

    if (mknod(FIFO_OUT_PATH, 0666, 0) < 0) {
        perror("Failed to create pipe");
        unlink_pipes();
        exit(EXIT_FAILURE);
    }

    int fd = open(FIFO_IN_PATH, O_WRONLY);
    if (fd == -1) {
        perror("open");
        unlink_pipes();
        exit(EXIT_FAILURE);
    }

    double a = strtod(argv[1], NULL);
    double b = strtod(argv[2], NULL);
    long n = strtol(argv[3], NULL, 10);

    if (write(fd, &a, sizeof(double)) < 0 ||
        write(fd, &b, sizeof(double)) < 0 ||
        write(fd, &n, sizeof(long)) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    close(fd);

    fd = open(FIFO_OUT_PATH, O_RDONLY);
    if (fd == -1) {
        perror("open");
        unlink_pipes();
        exit(EXIT_FAILURE);
    }

    double result = 0.0;

    // wait until manager writes calculated value to the output file
    while (!read(fd, &result, sizeof(double)));

    close(fd);

    printf("%f\n", result);
    unlink_pipes();

    return 0;
}
