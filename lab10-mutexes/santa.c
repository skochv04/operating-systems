#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define REINDEERS 9
#define DAYS 4

// santa and reindeers threads
pthread_t santa;
pthread_t reindeers[REINDEERS];

// mutex and condition variable
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// conditional variable
int reindeer_count = 0;

void* santa_handler(void* arg){
    // repeat for each day
    for(int i = 0; i < DAYS; i++) {
        pthread_mutex_lock(&mutex);
        printf("Mikołaj: zasypiam\n");

        // wait while reindeer_count is less than reindeers amount
        while (reindeer_count < REINDEERS) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("Mikołaj: budzę się\n");

        // deal when all reindeers are ready
        printf("Mikołaj: dostarczam zabawki\n");
        sleep(rand() % 3 + 2);
        reindeer_count = 0;

        // releases the deer
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void* reindeer_handler(void* arg){

    int id = *((int*)arg);
    // allow threads to cancel thread immediately
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while(1) {
        // sleep for 5-10 seconds
        sleep(rand() % 6 + 5);

        pthread_mutex_lock(&mutex);
        reindeer_count++;
        printf("Renifer: czeka %d reniferów na Mikołaja, ID: %d\n", reindeer_count, id);
        if (reindeer_count == REINDEERS){
            printf("Renifer: wybudzam Mikołaja, ID %d\n", id);

            // notify santa that condition is done
            pthread_cond_signal(&cond);
        }

        // wait until santa release reindeers
        pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex);
        if (reindeer_count == 0) {
            printf("Renifer: lecę na wakacje, %d\n", id);
        }
    }
}

int main(){
    int ids[REINDEERS];

    // create santa thread
    pthread_create(&santa, NULL, santa_handler, NULL);

    // create reindeer's threads
    for(int i = 0; i < REINDEERS; i++){
        ids[i] = i + 1;
        pthread_create(&reindeers[i], NULL, reindeer_handler, &ids[i]);
    }

    // wait for the end of santa and reindeer threads
    pthread_join(santa, NULL);

    for (int i = 0; i < REINDEERS; i++) {
        pthread_cancel(reindeers[i]);
        pthread_join(reindeers[i], NULL);
    }

    // clean up resources
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
