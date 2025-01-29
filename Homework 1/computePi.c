/*

To run:
gcc -o computePi computePi.c -lm -pthread
./a.out 10  # Example with 10 threads

 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>
#include <pthread.h>

#define STEPS 100

double step;      // The width of the step
double sum = 0.0; // Sum of areas
int np;           // Number of threads

pthread_mutex_t mutex; // Mutex for synchronization

// Timer function
double read_timer()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec * 1.0e-6;
}

// Funcations f(x) represent the upper part of the unit circle
double f(double x)
{
    return sqrt(1 - x * x);
}

/* The work the workers/threads will execute */
void *compute_pi_worker(void *arg)
{
    int worker_id = *(int *)arg;
    double local_sum = 0.0;

    // Define the range of the thread
    int start = worker_id * (STEPS / np);
    int end = (worker_id + 1) * (STEPS / np);

    for (int i = start; i < end; i++)
    {
        double x = i * step;
        local_sum += (f(x) + f(x + step)) * step / 2;
    }

    // Lock the mutex before updating the global sum
    pthread_mutex_lock(&mutex);

    sum += local_sum;

    // Unlock the mutex
    pthread_mutex_unlock(&mutex);

    free(arg); // Free the memory allocated for the thread id

    return NULL;
}

/*  */
int main(int argc, char *argv[])
{
    /* Too handle wrong amout of thread usages */
    if (argc != 2) // Check if the user entered the number of threads
    {
        printf("Usage: %s <number_of_threads>\n", argv[0]); // Print the usage of threads
        return 1;
    }
    /* To handle user error */
    np = atoi(argv[1]); // Number of threads
    if (np <= 0)        // Number of threads must be greater than 0
    {
        printf("Number of threads must be greater than 0.\n");
        return 1;
    }

    pthread_t worker_threads[np];       // Array of threads
    pthread_mutex_init(&mutex, NULL);   // Initialize the mutex
    step = 1.0 / STEPS;                 // Calculate the width of the step

    /* Start timer */
    double start_time = read_timer();

    // Create worker threads
    for (int i = 0; i < np; i++)
    {
        int *worker_id = malloc(sizeof(int));
        *worker_id = i;
        pthread_create(&worker_threads[i], NULL, compute_pi_worker, worker_id);
    }

    // Join worker threads
    for (int i = 0; i < np; i++)
    {
        pthread_join(worker_threads[i], NULL);
    }

    /* End timer */
    double end_time = read_timer();

    // Print the estimated value of Pi
    printf("Estimated Pi: %.15f\n", sum * 4);
    printf("Execution Time: %.6f seconds\n", end_time - start_time);

    pthread_mutex_destroy(&mutex); // Destroy the mutex

    return 0;
}
