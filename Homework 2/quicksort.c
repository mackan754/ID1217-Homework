#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define SIZE 1000000
#define THREADS 10

void swap(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int partition(int arr[], int low, int high) {
    
    int pivot = arr[high];
    
    int i = low - 1;

    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
  
    swap(&arr[i + 1], &arr[high]);  
    return i + 1;
}

void quickSort(int arr[], int low, int high) {
    if (low < high) {
        
        int pi = partition(arr, low, high);

        #pragma omp parallel 
        {
            #pragma omp sections 
            {
                quickSort(arr, low, pi - 1);
            }

            #pragma omp sections 
            {
                quickSort(arr, pi + 1, high);
            }
        }
    }
}

int main() {

    omp_set_num_threads(THREADS);
    srand(time(NULL));

    //Skapa arrayen och lägg in slumpmässiga värden.
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = rand() % 999;
    }

    //Påbörja quickSort.
    double t1 = omp_get_wtime();
    quickSort(arr, 0, SIZE - 1);
    double t2 = omp_get_wtime();

    double t = t2-t1;

    printf("%f", t);

    //Printar arrayen.
    /* for (int i = 0; i < SIZE; i++) {
        printf("%d ", arr[i]);
    } */
    
    return 0;
}