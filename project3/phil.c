/*
    @authors Muhammed Maruf Satir 21702908, Erdem Ege Eroğlu 21601636
    Operating system project 3
*/


#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <limits.h>


enum {THINKING, HUNGRY, EATING} state[5];
pthread_cond_t condVar[5];
pthread_mutex_t mutex;
int tpid[5];
pthread_t philosophers[5];

void test(int trNumber)
{
    if(state[(trNumber + 1) % 5] != EATING && state[(trNumber + 4) % 5] != EATING && state[trNumber] == HUNGRY)
    {
        state[trNumber] = EATING;
        pthread_cond_signal(&condVar[trNumber]);
    }
}

// i take the functions from the slides and extra
// implemented mutex lock
void putdown(int trNumber)
{
    pthread_mutex_lock(&mutex);
    printf("Philosopher %d finished eating now.\n", trNumber);
    state[trNumber] = THINKING;
    
    test((trNumber + 4) % 5);
    test((trNumber + 1) % 5);
    pthread_mutex_unlock(&mutex);  
}



void pickup(int trNumber)
{
    pthread_mutex_lock(&mutex);
    state[trNumber] = HUNGRY;
    test(trNumber);
    if(state[trNumber] != EATING)
    {
        pthread_cond_wait(&condVar[trNumber], &mutex);
    }

    printf("Philosopher %d started eating now.\n", trNumber);
    pthread_mutex_unlock(&mutex);
    
}

// just sleeping no need for mutex
void thinking(unsigned int seed)
{
    sleep((int)(rand_r(&seed) % 10 + 1));
}

void *phil(void *threadNumber)
{
    int trNumber = *((int*)threadNumber);
    unsigned int seed = pthread_self() % INT_MAX;

    while(1)
    {
        thinking(seed);

        pickup(trNumber);

        sleep((int)(rand_r(&seed) % 5 + 1));

        putdown(trNumber);
    }
    
}
int main()
{
    pthread_mutex_init(&mutex, NULL);
   
    for(int i = 0; i < 5; i++)
    {
        state[i] = THINKING;
        tpid[i] = i;
        pthread_cond_init(&condVar[i], NULL);
    }

        for(int i = 0; i < 5; i++)
        {
                    pthread_create(&philosophers[i], 0, phil, (void *)&tpid[i]);
        }

        // even this does not work because of while loop i wrote this 
        for(int i = 0; i < 5; i++)
        {
            pthread_join(philosophers[i], NULL);
        }
  
}