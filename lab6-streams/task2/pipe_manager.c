#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <sys/wait.h>

#define FIFO_IN_PATH "build/pipe_in"
#define FIFO_OUT_PATH "build/pipe_out"

double min(double a, double b){
    return a < b ? a : b;
}

double f(double x){
    return 4 / ((x * x) + 1);
}

double calculate_integral(double a, double b){
    double x = b - a;
    return x * f(a + x / 2);
}

// program works with arguments as a task1, and without arguments as a task2, when pipe_sender is working too
int main(int argc, char** argv){
    int fd_fifo = open(FIFO_IN_PATH, O_RDONLY);

    // integral left and right border, width of the rectangular to calculate
    double x1, x2, width;

    // number of processes to calculate integral separately
    long n;

    if (fd_fifo != -1){
        // part for case running with arguments from pipe_sender
        read(fd_fifo, &x1, sizeof(double));
        read(fd_fifo, &x2, sizeof(double));
        read(fd_fifo, &n, sizeof(long));
        width = (x2 - x1) / n;
    }
    else{
        // part for case running with arguments from command line
        // we need 2 argument to start: the source file, width of each range and amount of ranges (processes)
        if (argc < 3) {
            printf("Can not proceed, not enough arguments\n");
            return 1;
        }

        x1 = 0.0;
        x2 = 1.0;

        width = strtod(argv[1], NULL);
        n = strtol(argv[2], NULL, 10);
        printf("x1: %f, x2: %f, n: %ld, width: %f\n", x1, x2, n, width);
        if(ceil(((x2 - x1)/width)) < n) {
            printf("Not enough intervals for given processes number\n");
            return -1;
        }
    }

    // buffer to communicate through the pipe between parent and children
    int fd[n][2];

    for (int i = 0; i < n; i++){
        if(pipe(fd[i]) < 0){ printf("Failed to pipe");}
        pid_t pid = fork();
        if (pid == 0) {
            // child process
            close(fd[i][0]);
            printf("%fA\n", x1 + (double) width * i);
            printf("%fB\n", min(x1 + width *  (double) (i + 1), x2));
            double integral = calculate_integral(x1 + (double) width * i, min(x1 + width *  (double) (i + 1), x2));
            if (write(fd[i][1], &integral, sizeof(double)) < 0){
                printf("Failed to write");
                return -1;
            }

            exit(0);
        }
        // parent process
        close(fd[i][1]);
    }

    // parent waits until all children processes will be ended
    while (wait(NULL) > 0);

    double result = 0.0;

    for (int i = 0; i < n; i++){
        double buff;
        if (read(fd[i][0], &buff, sizeof(double)) < 0){
            printf("Failed to read data");
            return -1;
        }
        result += buff;
    }

    printf("Left boundary x1: %f\nRight boundary x2: %f\nRectangle width: %f\nNumber of intervals n: %ld\n", x1, x2, width, n);
    printf("\nIntegration result: %f\n", result);

    // part for case running with arguments from pipe_sender
    if(fd_fifo != -1){
        close(fd_fifo);
        fd_fifo = open(FIFO_OUT_PATH, O_WRONLY);
        if (fd_fifo == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        write(fd_fifo, &result, sizeof(double));
        close(fd_fifo);
    }

    return 0;
}

