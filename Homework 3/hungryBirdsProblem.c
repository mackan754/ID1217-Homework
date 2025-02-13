/*
   Compile and run:
    gcc hungryBirdsProblem.c -o hungryBirdsProblem -lpthread
    ./hungryBirdsProblem 10 2 // 10 worms and 2 baby birds
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

#define SHARED 1 // 1 means shared between threads, 0 means not shared. In this code we only have one process so it does not matter.
#define MAXBABYBIRDS 20
#define W 100

/* Semaphores */
sem_t dishLock;     // binary semaphore to protect dish access
sem_t parentWaits;  // parent waits on this semaphore to be signaled
sem_t dishRefilled; // baby birds waiting for dish refill wait on this

/* Shared variables */
int worms = W;              // current worms in the dish
int waitingCount = 0;       // number of baby birds waiting for a refill
bool callingParent = false; // flag to ensure only one baby bird calls(signals) to parent

void *BabyBird(void *arg) // Many Consumers
{
    long consumerID = (long)arg;

    printf("Baby bird %ld created\n", consumerID);

    while (true) // forver loop, loop as long that programs runs
    {
        // Simulate delay between eating attempts.
        sleep(1);

        sem_wait(&dishLock);

        /* If dish is empty */
        if (worms == 0)
        {
            waitingCount++;
            if (!callingParent)
            {
                callingParent = true;
                printf("Baby bird %ld finds no food and calls parent\n", consumerID);
                sem_post(&parentWaits); // signal and wake up parent
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
        sleep(1);
    }
}

void *ParentBird(void *arg) // One Producer
{
    printf("Parent bird created\n");

    while (true) // forever loop, loop as long that programs runs
    {
        // Wait till the baby birds call the parent that the dish is empty
        sem_wait(&parentWaits);

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
        sleep(1);
    }
}

int main(int argc, char *argv[])
{
    int numOfBabyBirds; // number of baby birds

    /* Read command line arguments, if any, else set to max */
    worms = (argc > 1) ? atoi(argv[1]) : W;
    numOfBabyBirds = (argc > 2) ? atoi(argv[2]) : MAXBABYBIRDS;

    if (worms > W)
        worms = W;

    if (numOfBabyBirds > MAXBABYBIRDS)
        numOfBabyBirds = MAXBABYBIRDS;

    printf("main started\n");

    /* Initilized semaphores */
    sem_init(&dishLock, SHARED, 1);
    sem_init(&dishRefilled, SHARED, 0);
    sem_init(&parentWaits, SHARED, 0);

    /* thread ids and attributes */
    pthread_t BabyBirdID[MAXBABYBIRDS];
    pthread_t ParentBirdID;

    /* Create Bird family */

    // Create one bird parent worker and start singel gathering
    pthread_create(&ParentBirdID, NULL, ParentBird, NULL);

    // Create many baby workers and start parlell eating
    for (long l = 0; l < numOfBabyBirds; l++)
    {
        pthread_create(&BabyBirdID[l], NULL, BabyBird, (void *)l);
    }

    /* Join Threads (aka kill bird family) */

    // Terminate parent worker after finish gathering
    pthread_join(ParentBirdID, NULL);

    // Terminate baby workers after finish eating
    for (long l = 0; l < numOfBabyBirds; l++)
    {
        pthread_join(BabyBirdID[l], NULL);
    }

    printf("main done\n");
    return 0;
}
