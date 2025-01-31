/*
To compile and run:
    gcc -o computePi computePi.c -lm -pthread
    ./computePi 20  # Example with 20 threads, we did not see an improvement with more than 20 threads
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>
#include <pthread.h>

#define STEPS 100000 // max 10_000_000_000

double step;      // The width of the step
double sum = 0.0; // Sum of areas
int np;           // Number of threads

pthread_mutex_t sumLock; // Mutex for synchronization

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

double integrate(double x)
{
    return (f(x) + f(x + step)) * step / 2;
}

/* The work the workers/threads will execute */
void *compute_pi_worker(void *arg)
{
    long worker_id = (long)arg;
    double local_sum = 0.0;

    // Define the range of the thread
    int start = worker_id * (STEPS / np);
    int end = (worker_id + 1) * (STEPS / np);

    for (int i = start; i < end; i++)
    {
        double x = i * step;
        local_sum += integrate(x);
    }

    // Lock the mutex before updating the global sum
    pthread_mutex_lock(&sumLock);

    sum += local_sum;

    // Unlock the mutex
    pthread_mutex_unlock(&sumLock);

    return NULL;    // Return NULL to avoid warnings
}

int main(int argc, char *argv[])
{
    /* Too handle user error: wrong amout/type of thread input */
    if (argc != 2) // Check if the user has entered the a correct number of arguments
    {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }

    np = atoi(argv[1]);

    /* To handle user error: input not a postive integer */
    if (np <= 0)
    {
        printf("Number of threads must be greater than 0.\n");
        return 1;
    }

    pthread_t worker_threads[np]; // Array of threads
    pthread_attr_t attr;
    pthread_mutex_init(&sumLock, NULL); // Initialize the mutex
    step = 1.0 / STEPS;                 // Calculate the width of the step
    long l;                          /* use long in case of a 64-bit system */

    /* set global thread attributes */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    /* Start timer */
    double start_time = read_timer();

    // Create worker threads and assign work
    for (l = 0; l < np; l++)
    {
        pthread_create(&worker_threads[l], &attr, compute_pi_worker, (void *)l);
    }

    // Join worker threads when they are done
    for (int i = 0; i < np; i++)
    {
        pthread_join(worker_threads[i], NULL);
    }

    /* End timer */
    double end_time = read_timer();

    // Print the estimated value of Pi
    // sum now holds the integral from x=0 to x=1 of sqrt(1 - x^2),
    // which is Pi/4. So multiply by 4 to get Pi.
    printf("Estimated Pi: %.15f\n", sum * 4);
    printf("Execution Time: %.6f seconds\n", end_time - start_time);

    return 0; // 0 = C program ran successfully
}
