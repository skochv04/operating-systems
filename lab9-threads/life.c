#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

int area = GRID_WIDTH * GRID_HEIGHT;

typedef struct{
    int left_border;
    int right_border;

    char **foreground;
    char **background;
} thread_args;

void alarm_handler(int signum) { } //the signal is used not to crash program but only "wake up" after pause()

int minInt(int a, int b){if (a < b) return a; else return b;}

void* thread_function(void *arg) {
    // dereference structure
    thread_args* args = (thread_args*)arg;

    while (true){
        // pause and wait for SIGALRM
        pause();

        for (int i = args->left_border; i < args->right_border; i++)
        {
            int row = i / GRID_WIDTH;
            int column = i % GRID_WIDTH;

            // update part of grid which this thread is responsible for
            (*args->background)[i] = is_alive(row, column, *args->foreground);
        }
    }
}

int main(int argc, char** argv)
{
    // we need 2 argument to start: the source file and amount of threads
    if (argc < 2) {
        printf("Can not proceed, not enough arguments\n");
        return 1;
    }

    // convert amount of threads to long value
    long threads_amount = strtol(argv[1], NULL, 10);

    srand(time(NULL));
	setlocale(LC_CTYPE, "");
	initscr(); // Start curses mode

	char *foreground = create_grid();
	char *background = create_grid();
	char *tmp;

	init_grid(foreground);

    // register handler
    signal(SIGALRM, alarm_handler);

    // create an array for threads
    pthread_t thread_list[threads_amount];

    // we need to keep args in array, because local variables would be destroyed after loop and thread would have no access to them
    thread_args args_list[threads_amount];

    // calculate the step
    int step = (int)ceil(area / threads_amount);

    // initialize each thread args and create threads
    for (int i = 0; i < threads_amount; i++){
        args_list[i].background = &background;
        args_list[i].foreground = &foreground;

        // calculate borders for this thread to calculate
        args_list[i].left_border = i * step;
        args_list[i].right_border = minInt((i + 1) * step, area);

        if (pthread_create(&thread_list[i], NULL, thread_function, &args_list[i])) perror("pthread_create");
    }


    while (true)
	{
		draw_grid(foreground);

        for(int i = 0; i < threads_amount; i++){
            pthread_kill(thread_list[i], SIGALRM);
        }

		usleep(500 * 1000);

		tmp = foreground;
		foreground = background;
		background = tmp;
	}

	endwin(); // End curses mode
	destroy_grid(foreground);
	destroy_grid(background);

	return 0;
}