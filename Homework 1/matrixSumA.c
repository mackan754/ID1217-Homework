/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc -o matrixSumA matrixSumA.c -lpthread
     ./matrixSumA 1000 4 [size numWorkers]

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
#include <limits.h> // new! For INT_MIN and INT_MAX

#define MAXSIZE 10000 /* maximum matrix size */
#define MAXWORKERS 10 /* maximum number of workers */

pthread_mutex_t barrier; /* mutex lock for the barrier */
pthread_cond_t go;       /* condition variable for leaving */
int numWorkers;          /* number of workers */
int numArrived = 0;      /* number who have arrived */

/* a reusable counter barrier */
void Barrier()
{
  pthread_mutex_lock(&barrier);
  numArrived++;
  if (numArrived == numWorkers)
  {
    numArrived = 0;
    pthread_cond_broadcast(&go);
  }
  else
    pthread_cond_wait(&go, &barrier);
  pthread_mutex_unlock(&barrier);
}

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
int sums[MAXWORKERS];         /* partial sums */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

typedef struct
{
  int i;
  int j;
  int value;
} Element;

Element workerMin[MAXWORKERS]; /* used for storing min values for different workers*/
Element workerMax[MAXWORKERS]; /* used for max values*/

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

  /* initialize mutex and condition variable */
  pthread_mutex_init(&barrier, NULL);
  pthread_cond_init(&go, NULL);

  /* read command line args if any */
  size = (argc > 1) ? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2) ? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE)
    size = MAXSIZE;
  if (numWorkers > MAXWORKERS)
    numWorkers = MAXWORKERS;
  stripSize = size / numWorkers;

  /* srand(time(NULL)); // seed to get different random values.. */

  /* initialize the matrix */
  for (i = 0; i < size; i++)
  {
    for (j = 0; j < size; j++)
    {
      matrix[i][j] = rand() % 99; // Random values between 0 and 98
    }
  }

  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++)
  {
    printf("[ ");
    for (j = 0; j < size; j++)
    {
      printf(" %d", matrix[i][j]);
    }
    printf(" ]\n");
  }
#endif

  /* Start timer */
  start_time = read_timer();

  /* do the parallel work: create the workers */
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *)l);
  pthread_exit(NULL);
}

/* Each worker computes the sum, max, and min of the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total, global max, and min. */
void *Worker(void *arg)
{
  long myid = (long)arg;
  int total, i, j, first, last;

  Element max = {0, 0, INT_MIN}; // new! Initialize max to the smallest possible value
  Element min = {0, 0, INT_MAX}; // new! Initialize min to the largest possible value

#ifdef DEBUG
  printf("worker %ld (pthread id %ld) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  first = myid * stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  /* compute sum, max, and min in my strip */
  total = 0;
  for (i = first; i <= last; i++)
  {
    for (j = 0; j < size; j++)
    {
      total += matrix[i][j];
      if (matrix[i][j] > max.value) // new! Check if the current value is greater than the current max
      {
        max.value = matrix[i][j];
        max.i = i;
        max.j = j;
      }
      if (matrix[i][j] < min.value) // new! Check if the current value is less than the current min
      {
        min.value = matrix[i][j];
        min.i = i;
        min.j = j;
      }
    }
  }

  /* new! Store results for this worker */
  sums[myid] = total;
  workerMax[myid] = max;
  workerMin[myid] = min;

  Barrier();

  /* worker 0 updates the global variables */
  if (myid == 0)
  {
    total = 0;

    for (i = 0; i < numWorkers; i++)
    {

      total += sums[i];

      if (workerMax[i].value > max.value) // new! Check if the current max is greater than the global max
      {
        max = workerMax[i];
      }
      if (workerMin[i].value < min.value) // new! Check if the current min is less than the global min
      {
        min = workerMin[i];
      }
    }

    /* get end time */
    end_time = read_timer();

    /* print results */
    printf("The total is %d\n", total);
    printf("The max value is %d at coordinates (%d, %d)\n", max.value, max.i, max.j);
    printf("The min value is %d at coordinates (%d, %d)\n", min.value, min.i, min.j);
    printf("The execution time is %g sec\n", end_time - start_time);
  }
}
