/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     ./a.out size numWorkers

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
#include <limits.h>   //för att använda INT_MIN och INT_MAX
#define MAXSIZE 10000 /* maximum matrix size */
#define MAXWORKERS 10 /* maximum number of workers */
// #define DEBUG

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

int threadMin[MAXWORKERS];
int threadMax[MAXWORKERS];
int threadMinPos[MAXWORKERS][2];
int threadMaxPos[MAXWORKERS][2];

/*
typedef struct {
  int partial_sum;
  int local_min;
  int local_max;
  int local_min_pos[2];
  int local_max_pos[2];
} ThreadResult; */

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

  /* initialize the matrix */
  srand(time(NULL)); // seed för att få olika värden varje gång programmet körs.

  for (i = 0; i < size; i++)
  {
    for (j = 0; j < size; j++)
    {
      matrix[i][j] = rand() % 99;
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

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *)l);
  pthread_exit(NULL);
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg)
{
  long myid = (long)arg;
  int total, i, j, first, last;

#ifdef DEBUG
  printf("worker %ld (pthread id %ld) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  first = myid * stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  /* sum values in my strip */
  total = 0;

  for (i = first; i <= last; i++)
    for (j = 0; j < size; j++)
      total += matrix[i][j];

  sums[myid] = total;

  /*
   * Fråga a.
   */

  int maxVal = INT_MIN;
  int minVal = INT_MAX;
  int maxValPos[2];
  int minValPos[2];

  for (i = first; i <= last; i++)
  {
    for (j = 0; j < size; j++)
    {
      if (maxVal < matrix[i][j])
      {
        maxVal = matrix[i][j]; // sparar det största värdet i matrisen i maxValue.
        maxValPos[0] = i;      // sparar positionen för det största värdet i matrisen.
        maxValPos[1] = j;      // sparar positionen för det största värdet i matrisen.
      }
      if (minVal > matrix[i][j])
      {
        minVal = matrix[i][j]; // sparar det minsta värdet i matrisen i minValue.
        minValPos[0] = i;      // sparar det minsta värdet i matrisen i minValue.
        minValPos[1] = j;      // sparar positionen för det minsta värdet i matrisen.
      }
    }
  }

  threadMin[myid] = minVal;             // sparar det minsta värdet i varje tråd.
  threadMinPos[myid][0] = minValPos[0]; // sparar column positionen för det minsta värdet i varje tråd.
  threadMinPos[myid][1] = minValPos[1]; // sparar rad positionen för det minsta värdet i varje tråd.

  threadMax[myid] = maxVal;             // sparar det största värdet i varje tråd.
  threadMaxPos[myid][0] = maxValPos[0]; // sparar column positionen för det största värdet i varje tråd.
  threadMaxPos[myid][1] = maxValPos[1]; // sparar rad positionen för det största värdet i varje tråd.

  Barrier();

  if (myid == 0)
  {

    total = 0;

    for (i = 0; i < numWorkers; i++)
    {
      total += sums[i];
    }

    int globalMax = INT_MIN;
    int globalMin = INT_MAX;
    int globalMaxPos[2];
    int globalMinPos[2];

    for (int w = 0; w < numWorkers; w++)
    {

      if (threadMax[w] > globalMax)
      {
        globalMax = threadMax[w];
        globalMaxPos[0] = threadMaxPos[w][0];
        globalMaxPos[1] = threadMaxPos[w][1];
      }

      if (threadMin[w] < globalMin)
      {
        globalMin = threadMin[w];
        globalMinPos[0] = threadMinPos[w][0];
        globalMinPos[1] = threadMinPos[w][1];
      }
    }

    /* get end time */
    end_time = read_timer();

    /* print results */
    printf("The total is %d\n", total);
    printf("The execution time is %g sec\n", end_time - start_time);
    printf("Global Max = %d at position (%d, %d)\n",
           globalMax, globalMaxPos[0], globalMaxPos[1]);
    printf("Global Min = %d at position (%d, %d)\n",
           globalMin, globalMinPos[0], globalMinPos[1]);
  }
}