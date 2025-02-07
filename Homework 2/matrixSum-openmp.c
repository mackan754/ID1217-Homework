/* matrix summation using OpenMP

   usage with gcc (version 4.2 or higher required):
     gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c
     ./matrixSum-openmp size numWorkers

*/

#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

double start_time, end_time;

#define MAXSIZE 10000 /* maximum matrix size */
#define MAXWORKERS 8  /* maximum number of workers */

int numWorkers;
int size;
int matrix[MAXSIZE][MAXSIZE];

// void *Worker(void *); //behöver vi använda den här worker?

/* read command line, initialize, and create threads */
int main(int argc, char *argv[])
{
  int i, j, total = 0;

  /* read command line args if any */
  size = (argc > 1) ? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2) ? atoi(argv[2]) : MAXWORKERS;

  if (size > MAXSIZE)
    size = MAXSIZE;
  if (numWorkers > MAXWORKERS)
    numWorkers = MAXWORKERS;

  omp_set_num_threads(numWorkers);

  /* initialize the matrix */
  for (i = 0; i < size; i++)
  {
    // printf("[ ");
    for (j = 0; j < size; j++)
    {
      matrix[i][j] = rand() % 99;
      // printf(" %d", matrix[i][j]);
    }
    // printf(" ]\n");
  }

  // Global result variables
  int globalMax = INT_MIN, globalMin = INT_MAX;                       // new! Initialize globalMax to the smallest possible value and globalMin to the largest possible value
  int globalMaxI = 0, globalMaxJ = 0, globalMinI = 0, globalMinJ = 0; // new! Initialize globalMax and globalMin coordinates to 0

  start_time = omp_get_wtime();

#pragma omp parallel for reduction(+ : total) private(j)
  for (i = 0; i < size; i++)
  {
    // Local result variables
    int localMin = INT_MAX, localMax = INT_MIN;                     // new! Initialize local min to the largest possible value and local max to the smallest possible value
    int localMaxI = 0, localMaxJ = 0, localMinI = 0, localMinJ = 0; // new! Initialize local max and min coordinates to 0
    
    for (j = 0; j < size; j++)
    {
      int matrixValue = matrix[i][j]; // new! Store the value of the current element in a variable for easier understanding
      total += matrixValue;

      if (localMax < matrixValue)
      {
        localMax = matrixValue;
        localMaxI = i;
        localMaxJ = j;
      }
      if (localMin > matrixValue)
      {
        localMin = matrixValue;
        localMinI = i;
        localMinJ = j;
      }
    }
#pragma omp critical
    {

      // Only one thread executes this block at a time.
      if (globalMax < localMax)
      {
        globalMax = localMax;
        globalMaxI = localMaxI;
        globalMaxJ = localMaxJ;
      }

      if (globalMin > localMin)
      {
        globalMin = localMin;
        globalMinI = localMinI;
        globalMinJ = localMinJ;
      }
    }
  }

  end_time = omp_get_wtime();

  printf("the total is %d\n", total);
  printf("it took %g seconds\n", end_time - start_time);
  printf("The max value is %d at coordinates (%d, %d)\n", globalMax, globalMaxI, globalMaxJ);
  printf("The min value is %d at coordinates (%d, %d)\n", globalMin, globalMinI, globalMinJ);

  return 0; // Fuck you Marcus!
}