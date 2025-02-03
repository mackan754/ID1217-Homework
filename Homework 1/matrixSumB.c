/*
   usage under Linux:
     gcc matrixSumB.c -lpthread
     a.out size numWorkers
        gcc -o matrixSumB matrixSumB.c -lpthread
        ./matrixSumB 1000 4 [matrix size, number of Workers]
 */

#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>

#define MAXSIZE 10000 /* maximum matrix size */
#define MAXWORKERS 10 /* maximum number of workers */

pthread_mutex_t maxMinLock; /* new! mutex lock for checking maximum and minimum*/
pthread_mutex_t sumLock;    /* new! mutex lock for adding to total*/
int numWorkers;             /* number of workers */

/* timer */
double read_timer()
{
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if (!initialized)
    {
        gettimeofday(&start, NULL);
        initialized = true;
    }
    gettimeofday(&end, NULL);
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time;  /* start and end times */
int size, stripSize;          /* assume size is multiple of numWorkers */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

int max = INT_MIN, min = INT_MAX;           // new! Initialize max to the smallest possible value and min to the largest possible value
int maxI = 0, maxJ = 0, minI = 0, minJ = 0; // new! Initialize max and min coordinates to 0
int total = 0;                              // new! Initialize total to 0

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[])
{
    int i, j;
    long l; /* use long in case of a 64-bit system */
    pthread_attr_t attr;
    pthread_t workerid[MAXWORKERS];

    /* set global thread attributes */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    /* initialize mutex for maximum and minimum and total sum*/
    pthread_mutex_init(&maxMinLock, NULL);
    pthread_mutex_init(&sumLock, NULL);

    /* read command line args if any */
    size = (argc > 1) ? atoi(argv[1]) : MAXSIZE;
    numWorkers = (argc > 2) ? atoi(argv[2]) : MAXWORKERS;
    if (size > MAXSIZE)
        size = MAXSIZE;
    if (numWorkers > MAXWORKERS)
        numWorkers = MAXWORKERS;
    stripSize = size / numWorkers;

    // srand(time(NULL)); // seed för att få olika värden varje gång programmet körs.

    /* initialize the matrix */
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            matrix[i][j] = rand() % 99;
        }
    }

    /* do the parallel work: create the workers */
    start_time = read_timer();

    for (l = 0; l < numWorkers; l++)
    {
        pthread_create(&workerid[l], &attr, Worker, (void *)l);
    }

    /* Worker threads do work... */

    /* join all threads */
    for (l = 0; l < numWorkers; l++)
    {
        pthread_join(workerid[l], NULL);
    }
    end_time = read_timer();

    /* print results */
    printf("The total is %d\n", total);
    printf("The max value is %d at coordinates (%d, %d)\n", max, maxI, maxJ);
    printf("The min value is %d at coordinates (%d, %d)\n", min, minI, minJ);
    printf("The execution time is %g sec\n", end_time - start_time);
}

void *Worker(void *arg)
{
    long myid = (long)arg;
    int localTotal = 0, i, j, first, last;

    int localMin = INT_MAX, localMax = INT_MIN;                     // new! Initialize local min to the largest possible value and local max to the smallest possible value
    int localMaxI = 0, localMaxJ = 0, localMinI = 0, localMinJ = 0; // new! Initialize local max and min coordinates to 0

    /* determine first and last rows of my strip */
    first = myid * stripSize;
    last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);
    /* 
    if (myid == numWorkers - 1)
    {
        last = size - 1;
    }
    else
    {
        last = first + stripSize - 1;
    }
    */

        /* compute sum, max, and min in my strip */
        for (i = first; i <= last; i++)
    {
        for (j = 0; j < size; j++)
        {
            localTotal += matrix[i][j];
            if (localMax < matrix[i][j])
            {
                localMax = matrix[i][j];
                localMaxI = i;
                localMaxJ = j;
            }
            if (localMin > matrix[i][j])
            {
                localMin = matrix[i][j];
                localMinI = i;
                localMinJ = j;
            }
        }
    }

    pthread_mutex_lock(&maxMinLock); // new! lock the mutex before checking the maximum and minimum

    if (max < localMax)
    {
        max = localMax;
        maxI = localMaxI;
        maxJ = localMaxJ;
    }

    if (min > localMin)
    {
        min = localMin;
        minI = localMinI;
        minJ = localMinJ;
    }

    pthread_mutex_unlock(&maxMinLock); // new! unlock the mutex after checking the maximum and minimum

    pthread_mutex_lock(&sumLock); // new! lock the mutex before adding to the total

    total += localTotal;

    pthread_mutex_unlock(&sumLock); // new! unlock the mutex after adding to the total
}