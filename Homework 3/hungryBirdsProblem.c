/* a simple parentBird/babyBird using semaphores and threads

   usage on Solaris:
    gcc hungryBirdsProblem.c -o hungryBirdsProblem -lpthread -lposix4
    ./hungryBirdsProblem numIters

*/
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define SHARED 1
#define MAXITERATION 100
#define MAXBABYBIRD 20
#define MAXWORMS 5 // Number of worms the parent will gather each time

void *ParentBird(void *); // One Producer
void *BabyBird(void *);   // Many Consumers

sem_t dishIsEmpty, dishIsFull, printSem; /* the global semaphores */
int worms = 0;                           /* shared buffer         */
int babyBirdHungry;
int numberOfBabyBirds; // new

/* main() -- read command line and create threads, then
             print result when the threads have quit */

int main(int argc, char *argv[])
{

    /* Read command line arguments, if any, else set to max */
    babyBirdHungry = (argc > 1) ? atoi(argv[1]) : 1;
    numberOfBabyBirds = (argc > 2) ? atoi(argv[2]) : MAXBABYBIRD;

    if (babyBirdHungry > MAXITERATION)
        babyBirdHungry = MAXITERATION;

    if (numberOfBabyBirds > MAXBABYBIRD)
        numberOfBabyBirds = MAXBABYBIRD;

    printf("main started\n");
    /* start timer here */

    /* Initilized semaphores */
    sem_init(&dishIsEmpty, SHARED, 1); /* sem dishIsEmpty = 1 */
    sem_init(&dishIsFull, SHARED, 0);  /* sem dishIsFull = 0  */
    sem_init(&printSem, SHARED, 1);    /* sem to control print order  */

    /* thread ids and attributes */
    pthread_t babyBirdID[MAXBABYBIRD];
    pthread_t parentBirdID;

    /* Create Bird family */

    // Create one bird parent worker and start singel gathering
    pthread_create(&parentBirdID, NULL, ParentBird, NULL);

    // Create many baby workers and start parlell eating
    for (long l = 0; l < numberOfBabyBirds; l++)
    {
        pthread_create(&babyBirdID[l], NULL, BabyBird, (void *)l);
    }

    /* Join Threads (aka kill bird family) */

    // Terminate parent worker after finish gathering
    pthread_join(parentBirdID, NULL);

    // Terminate baby workers after finish eating
    for (long l = 0; l < numberOfBabyBirds; l++)
    {
        pthread_join(babyBirdID[l], NULL);
    }

    printf("main done\n");

    /* end timer here */
    return 0;
}

/* deposit 1, ..., numIters into the data buffer */
void *ParentBird(void *arg) // One Producer
{
    printf("Parent bird created\n");

    for (int gatherWorms = 0; gatherWorms < babyBirdHungry; gatherWorms++)
    {
        if (worms == 0) // Added to handle deadlock
        {
            // If no worms are left and all iterations are complete, stop refilling
            sem_post(&dishIsFull); // Signal to baby birds that the dish is empty
            break;
        }
        sem_wait(&dishIsEmpty);                           // Wait for the dish to be empty
        worms = MAXWORMS;                                 // Gather worms
        sem_wait(&printSem);                              // Wait for the baby birds to finish eating
        printf("Parent bird gathered %d worms\n", worms); // Enuser that the parent bird is gathering worms
        sem_post(&printSem);                              // Signal the baby birds that the parent bird is done gathering
        sem_post(&dishIsFull);                            // Signal the baby birds that the dish is full
    }

    return NULL;
}

/* fetch numIters items from the buffer and sum them */
void *BabyBird(void *arg) // Many Consumers
{

    long birdID = (long)arg;
    int totalAmountOfWormsConsumed = 0, consumed;

    sem_wait(&printSem); // Ensure that only one thread prints at a time
    printf("Baby bird %ld created\n", birdID);
    sem_post(&printSem); // Release the print semaphore

    for (consumed = 0; consumed < babyBirdHungry; consumed++)
    {
        sem_wait(&dishIsFull);

        if (worms == 0)
        {
            // If no worms are left, exit the loop
            break;
        }
        totalAmountOfWormsConsumed += worms;
        sem_wait(&printSem); // Ensure that only one thread prints at a time
        printf("Baby bird %ld eats %d worms. Total eaten: %d\n", birdID, worms, totalAmountOfWormsConsumed);
        sem_post(&printSem); // Release the print semaphore
        sem_post(&dishIsEmpty);
    }

    sem_wait(&printSem); // Ensure that only one thread prints at a time
    printf("for %d iterations, the total Amount of worms consumed is %d\n", babyBirdHungry, totalAmountOfWormsConsumed);
    sem_wait(&printSem); // Ensure that only one thread prints at a time
    return NULL;
}
