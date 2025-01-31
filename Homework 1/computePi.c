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

<<<<<<< Updated upstream
<<<<<<< Updated upstream
#define STEPS 1000000000
=======
#define STEPS 1000000000 // max 10_000_000_000
>>>>>>> Stashed changes
=======
#define STEPS 1000000000 // max 10_000_000_000
>>>>>>> Stashed changes

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
}

int main(int argc, char *argv[])
{
<<<<<<< Updated upstream
<<<<<<< Updated upstream
    /* To handle user error */
    if (argc != 2) 
    {
        printf("Usage: %s <number_of_threads>\n", argv[0]); 
        return 1;
    }

    /* To handle user error */
    np = atoi(argv[1]); 
    if (np <= 0)        
=======
    /* Too handle wrong amout/type of thread input */
    if (argc != 2) // Check if the user has entered the a correct number of arguments
    {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }

=======
    /* Too handle wrong amout/type of thread input */
    if (argc != 2) // Check if the user has entered the a correct number of arguments
    {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }

>>>>>>> Stashed changes
    np = atoi(argv[1]); // Number of threads. ASCI to integer

    /* To handle user input error */
    if (np <= 0)        // Number of threads must be an positive integer
<<<<<<< Updated upstream
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
    {
        printf("Number of threads must be greater than 0.\n");
        return 1;
    }

<<<<<<< Updated upstream
<<<<<<< Updated upstream
    pthread_t worker_threads[np];       // Array of threads
    pthread_attr_t attr;
    pthread_mutex_init(&sumLock, NULL);   // Initialize the mutex
    step = 1.0 / STEPS;                 // Calculate the width of the step
     long l; /* use long in case of a 64-bit system */

    /* set global thread attributes */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
=======
    pthread_t worker_threads[np];     // Array of threads
    pthread_mutex_init(&mutex, NULL); // Initialize the mutex
    step = 1.0 / STEPS;               // Calculate the width of the step
>>>>>>> Stashed changes
=======
    pthread_t worker_threads[np];     // Array of threads
    pthread_mutex_init(&mutex, NULL); // Initialize the mutex
    step = 1.0 / STEPS;               // Calculate the width of the step
>>>>>>> Stashed changes

    /* Start timer */
    double start_time = read_timer();

    // Create worker threads
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
    printf("Estimated Pi: %.15f\n", sum * 4);
    printf("Execution Time: %.6f seconds\n", end_time - start_time);
}
