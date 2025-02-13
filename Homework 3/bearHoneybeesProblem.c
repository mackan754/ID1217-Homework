#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define MAXBEES 100
#define SHARED 1

void *Bee(void *);
void *Bear(void *);

sem_t spaceAvailable, honeyCount;
int honey;
int numBees;
int pot;

int main(int argc, char *argv[])
{
    /* thread ids and attributes */
    pthread_t bearId;
    pthread_t beeId[MAXBEES];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    /* Set default values */
    numBees = 10;
    pot = 10;

    if (argc > 1)
        numBees = atoi(argv[1]);
    if (argc > 2)
        pot = atoi(argv[2]);

    sem_init(&spaceAvailable, SHARED, pot);
    sem_init(&honeyCount, SHARED, 0);

    printf("main started\n");

    pthread_create(&bearId, &attr, Bear, NULL);

    for (long l = 0; l < numBees; l++)
    {
        pthread_create(&beeId[l], &attr, Bee, (void *)l);
    }

    for (long l = 0; l < numBees; l++)
    {
        pthread_join(beeId[l], NULL);
    }

    pthread_join(bearId, NULL);

    printf("main done\n");
}

/* deposit 1, ..., numIters into the data buffer */
void *Bee(void *arg)
{
    long myid = (long)arg;
    printf("Bee %ld created\n", myid);

    while (1)
    {
        sem_wait(&spaceAvailable);
        honey++;
        printf("Bee %ld deposited honey.\n", myid);

        sem_post(&honeyCount);

        usleep(1000000);
    }
}

/* Bear consumes honey when the pot is full (no empty spaces left) */
void *Bear(void *arg)
{
    while (1)
    {
        for (int i = 0; i < pot; i++)
        {
            sem_wait(&honeyCount); 
        }

        printf("Bear awakened. Consuming honey.\n");
        honey = 0;
        printf("Bear ate all the honey. Pot is now empty.\n");

        for (int i = 0; i < pot; i++)
        {
            sem_post(&spaceAvailable); 
        }

        usleep(1000000);
    }
}
