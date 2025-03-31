#include <stdio.h>

#ifndef DYNAMIC_DLOPEN
    #include "collatz_lib/collatz.h"
#endif

#ifdef DYNAMIC_DLOPEN
    #include "dlfcn.h"

    int (*collatz_conjecture)(int input);
    int (*test_collatz_convergence)(int input, int max_iter);
#endif

int main(){
//Dynamically Loaded Libraries
#ifdef DYNAMIC_DLOPEN
    void *handle = dlopen("collatz_lib/build/libcollatz_shared.so", RTLD_LAZY);
    if (!handle){
        fprintf(stderr, "Error: %s\n", dlerror());
        return 1;
    }

    collatz_conjecture = dlsym(handle, "collatz_conjecture");
    test_collatz_convergence = dlsym(handle, "test_collatz_convergence");
#endif

    printf("Result of collatz_conjecture(17): %d\n", collatz_conjecture(17));
    printf("Result of test_collatz_convergence(17, 100): %d\n", test_collatz_convergence(17, 100));

}