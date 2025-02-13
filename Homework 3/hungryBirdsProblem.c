/* a simple OneProducer/ManyConsumers using semaphores and threads

   usage on Solaris:
    gcc hungryBirdsProblem.c -o hungryBirdsProblem -lpthread
    ./hungryBirdsProblem numIters

*/
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <unistd.h>

#define SHARED 1
#define MAXITERATION 1000000
#define MAXCONSUMERS 2
#define W 10

void *OneProducer(void *);   // One Producer
void *ManyConsumers(void *); // Many Consumers

/* the global semaphores */
sem_t dishLock;     // binary semaphore to protect dish access
sem_t parentSem;    // parent waits on this semaphore to be signaled
sem_t dishRefilled; // baby birds waiting for dish refill wait on this

// Shared variables
int worms = W;              // current worms in the dish
int waitingCount = 0;       // number of baby birds waiting for a refill
bool callingParent = false; // flag to ensure only one call to parent

int main(int argc, char *argv[])
{
    int numOfConsumers; // number of baby birds
    /* Read command line arguments, if any, else set to max */
    worms = (argc > 1) ? atoi(argv[1]) : W;
    numOfConsumers = (argc > 2) ? atoi(argv[2]) : MAXCONSUMERS;

    if (worms > MAXITERATION)
        worms = MAXITERATION;

    if (numOfConsumers > MAXCONSUMERS)
        numOfConsumers = MAXCONSUMERS;

    printf("main started\n");

    /* Initilized semaphores */
    sem_init(&dishLock, SHARED, 1);
    sem_init(&dishRefilled, SHARED, 0);
    sem_init(&parentSem, SHARED, 0);

    /* thread ids and attributes */
    pthread_t manyConsumersID[MAXCONSUMERS];
    pthread_t OneProducerID;

    /* Create Bird family */

    // Create one bird parent worker and start singel gathering
    pthread_create(&OneProducerID, NULL, OneProducer, NULL);

    // Create many baby workers and start parlell eating
    for (long l = 0; l < numOfConsumers; l++)
    {
        pthread_create(&manyConsumersID[l], NULL, ManyConsumers, (void *)l);
    }

    /* Join Threads (aka kill bird family) */

    // Terminate parent worker after finish gathering
    pthread_join(OneProducerID, NULL);

    // Terminate baby workers after finish eating
    for (long l = 0; l < numOfConsumers; l++)
    {
        pthread_join(manyConsumersID[l], NULL);
    }

    printf("main done\n");
    // printf("for %d iterations, the total Amount of food consuming is %d\n", fullPot, worms);
    return 0;
}

/* deposit 1, ..., numIters into the data buffer */
void *OneProducer(void *arg) // One Producer
{
    printf("Parent bird created\n");
    while (1)
    {
        // Wait till the baby birds call the parent that the dish is empty
        sem_wait(&parentSem);

        // Refill dish with W food
        sem_wait(&dishLock);
        printf("Parent bird refills the dish with %d worms.\n", W);
        worms = W;

        while (waitingCount > 0)
        {
            sem_post(&dishRefilled);
            waitingCount--;
        }
        callingParent = false;
        sem_post(&dishLock);

        // Simulate the time to gather worms.
        sleep(rand() % 3 + 1);

        return NULL;
    }
}

/* fetch numIters items from the buffer and sum them */
void *ManyConsumers(void *arg) // Many Consumers
{
    long consumerID = (long)arg;
    printf("Baby bird %ld created\n", consumerID);

    while (1)
    {
        // Simulate delay between eating attempts.
        sleep(rand() % 3 + 1);

        sem_wait(&dishLock);
        if (worms == 0)
        {
            waitingCount++;
            if (!callingParent)
            {
                callingParent = true;
                printf("Baby bird %ld finds no food and calls parent\n", consumerID);
                sem_post(&parentSem); // signal and wake up parent
            }
            sem_post(&dishLock);
            sem_wait(&dishRefilled);
            continue; // go back to the top of the loop
        }

        worms--;
        printf("Baby bird %ld takes a worm. Worms left: %d\n", consumerID, worms);
        sem_post(&dishLock);

        // Simulate eating.
        printf("Baby bird %ld is eating a worm.\n", consumerID);
        sleep(rand() % 3 + 1);
    }
    return NULL;
}
