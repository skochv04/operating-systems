#include "collatz.h"

int collatz_conjecture(int input){
    if (input % 2 == 0){
        input /= 2;
    }
    else {
        input = 3 * input + 1;
    }
    return input;
}

int test_collatz_convergence(int input, int max_iter){
    int iterations = 0;
    while (iterations < max_iter && input != 1){
        iterations++;
        input = collatz_conjecture(input);
    }
    if (input == 1){
        return iterations;
    }
    else return -1;
}