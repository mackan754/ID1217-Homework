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

#define MAXSIZE 10000  
#define MAXWORKERS 10   

pthread_mutex_t bagLock; /* mutex lock for bag */
pthread_mutex_t maxMinLock; /* mutex lock for min and max value */
pthread_mutex_t sumLock; /* mutex lock for sum */
int numWorkers; /* number of workers*/

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if(!initialized) {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time; /* start and end times */
int size, stripSize; /* assume size is multiple of numWorkers */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */
int max = INT_MIN, min = INT_MAX;
int maxI = 0, maxJ = 0, minI = 0, minJ = 0; /* used for storing position of min and max */
int total = 0;

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
    int i, j;
    long l; /* use long in case of a 64-bit system */
    pthread_attr_t attr;
    pthread_t workerid[MAXWORKERS];

    /* set global thread attributes */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    /* initialize mutex for bag, maximum, minimum, total sum*/
    pthread_mutex_init(&bagLock, NULL);
    pthread_mutex_init(&maxMinLock, NULL);
    pthread_mutex_init(&sumLock, NULL);

    /* read command line args if any */
    size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
    numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
    if (size > MAXSIZE) size = MAXSIZE;
    if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
    stripSize = size/numWorkers;

     /* initialize the matrix */
    for (i = 0; i < size; i++) {
	    for (j = 0; j < size; j++) {
            matrix[i][j] = rand()%99;
	    }
    }

    /* do the parallel work: create the workers */
    start_time = read_timer();
    for (l = 0; l < numWorkers; l++) {
        pthread_create(&workerid[l], &attr, Worker, (void *)l);
    }

    /* join all threads */
    for (l = 0; l < numWorkers; l++) {
        pthread_join(workerid[l], NULL);
    }
    end_time = read_timer();

    /* print results */
    printf("The total is %d\n", total);
    printf("The max value is %d at coordinates (%d, %d)\n", max, maxI, maxJ);
    printf("The min value is %d at coordinates (%d, %d)\n", min, minI, minJ);
    printf("The execution time is %g sec\n", end_time - start_time);
}

void *Worker(void *arg) {
    long myid = (long)arg;
    int localTotal = 0, row = 0, j, nextRow = 0;
    int localMin = INT_MAX, localMax = INT_MIN;
    int localMaxI = 0, localMaxJ = 0, localMinI = 0, localMinJ = 0;

    /* loop will run until everything has been processed and then break*/
    while (true) {
        /* Each tread pick the next avaliable row.*/
        pthread_mutex_lock(&bagLock);
        row = nextRow;
        nextRow++;
        pthread_mutex_unlock(&bagLock);

        if(row >= size) {
            break;
        }

        for (j = 0; j < size; j++) {
            localTotal += matrix[row][j];

            if (localMax < matrix[row][j]) {
                localMax = matrix[row][j];
                localMaxI = row;
                localMaxJ = j;
            }
            if (localMin > matrix[row][j]) {
                localMin = matrix[row][j];
                localMinI = row;
                localMinJ = j;
            }
        }
    }

    /* lock before updating global variables*/
    pthread_mutex_lock(&maxMinLock);
    if (max < localMax) {
        max = localMax;
        maxI = localMaxI;
        maxJ = localMaxJ;
    }

    if (min > localMin) {
        min = localMin;
        minI = localMinI;
        minJ = localMinJ;
    }
    pthread_mutex_unlock(&maxMinLock);

    pthread_mutex_lock(&sumLock);
    total += localTotal;
    pthread_mutex_unlock(&sumLock);
}
