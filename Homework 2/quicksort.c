#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define SIZE 1500000
#define THREADS 4

void swapElements(int *x, int *y)
{
    int temp = *x;
    *x = *y;
    *y = temp;
}

int partitionArray(int arr[], int low, int high)
{

    int pivot = arr[high]; // Last element as pivot

    int i = low - 1; // Index of smaller element

    for (int j = low; j <= high - 1; j++)
    {
        if (arr[j] < pivot)
        {
            i++;
            swapElements(&arr[i], &arr[j]); // Swap elements if element is smaller than pivot
        }
    }

    swapElements(&arr[i + 1], &arr[high]); // Place pivot element in correct position
    return i + 1;                          // Return pivot index
}

void quickSort(int arr[], int low, int high)
{
    if (low < high)
    {

        int pi = partitionArray(arr, low, high); // Seperate array into partitions indexs

#pragma omp task //Parallelize the left side of the array   
        {
            quickSort(arr, low, pi - 1); // Sort left sida parallel
        }

#pragma omp task //Parallelize the right side of the array
        {
            quickSort(arr, pi + 1, high); // Sort right sida parallel
        }
#pragma omp taskwait // Wait for both tasks to finish
    }
}

int main()
{

    omp_set_num_threads(THREADS);
    srand(time(NULL));

    // Skapa arrayen och lägg in slumpmässiga värden.
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++)
    {
        arr[i] = rand() % 999;
    }

    double t1 = omp_get_wtime();

#pragma omp parallel //Start parallel region
    {
#pragma omp single // Only let one thread execute this block
        {
            quickSort(arr, 0, SIZE - 1);
        }
    }

    double t2 = omp_get_wtime();

    double t = t2 - t1;

    printf("%f", t);

    // Printar arrayen.
    /* for (int i = 0; i < SIZE; i++) {
        printf("%d ", arr[i]);
    } */

    return 0;
}