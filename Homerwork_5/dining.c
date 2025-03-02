/*
mpicc -o dining dining.c
mpirun --oversubscribe -np 6 ./dining
*/

/*
 * LÖSNINGENS ÖVERSIKT:
 *
 * 1. Antal processer: 6 (rank 0..4 = Filosofer, rank 5 = Server).
 * 2. Servern (bordet) håller koll på fem "chopsticks". En chopstick är ledig
 *    om den är märkt med -1, annars är den uppplockad av en viss filosof.
 * 3. Filosoferna:
 *    - Varje filosof sover en slumpmässig tid för att "tänka".
 *    - Sedan skickar filosofen en PICKUP-förfrågan till servern. Servern
 *      svarar med GRANT om båda bestick är lediga eller WAIT om minst
 *      ett bestick är upptaget.
 *    - Vid GRANT sover filosofen ytterligare en slumpmässig tid för att "äta".
 *    - Därefter skickar filosofen PUTDOWN för att släppa sina bestick.
 * 4. Servern skriver ut "trace"-meddelanden (t.ex. beviljar bestick, tar emot
 *    PUTDOWN osv.) och filosoferna skriver ut när de tänker/äter/väntar.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Meddelandetaggar
#define PICKUP 1
#define PUTDOWN 2
#define GRANT 3
#define WAIT 4

// För enkelhet: 5 filosofer (ranks 0..4) + 1 server (rank 5)
#define NUM_PHILOSOPHERS 5
#define SERVER_RANK 5

// Array för att hålla koll på chopsticks:
// -1 betyder ledig, annars är indexet ranken på den filosof som håller i besticket.
int chopsticks[NUM_PHILOSOPHERS];

// Vänster/höger chopstick-funktioner
int left_chopstick(int phil)
{
    return phil;
}

int right_chopstick(int phil)
{
    return (phil + 1) % NUM_PHILOSOPHERS;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Vi vill ha 6 processer (5 filosofer + 1 server)
    if (world_size != 6)
    {
        if (world_rank == 0)
        {
            printf("Starta med 6 processer (5 filosofer + 1 server)!\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Varje process får ett unikt slumpfrö
    srand(time(NULL) + world_rank);

    if (world_rank < 5) // Bara filsofer har rank id under 5
    {
        int phil_id = world_rank;
        int think_time = rand() % 3 + 1;
        int eat_time = rand() % 3 + 1;
        int wait_time = rand() % 2 + 1;

        while (1)
        {
            // 1) Tänka
            printf("Filosof %d tänker i %d sek...\n", phil_id, think_time);
            fflush(stdout);
            sleep(think_time);

            // 2) Skicka PICKUP-begäran till server
            printf("Filosof %d: Begär att få plocka upp besticken.\n", phil_id);
            fflush(stdout);
            MPI_Send(NULL, 0, MPI_CHAR, SERVER_RANK, PICKUP, MPI_COMM_WORLD);

            // 3) Ta emot svar (GRANT eller WAIT)
            MPI_Status status;
            MPI_Recv(NULL, 0, MPI_CHAR, SERVER_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == GRANT)
            {
                // 4) Äta (slumpmässig tid)
                printf("Filosof %d fick besticken, äter i %d sek...\n", phil_id, eat_time);
                fflush(stdout);
                sleep(eat_time);

                // 5) Lägg ner besticken
                printf("Filosof %d lägger tillbaka besticken.\n", phil_id);
                fflush(stdout);
                MPI_Send(NULL, 0, MPI_CHAR, SERVER_RANK, PUTDOWN, MPI_COMM_WORLD);
            }
            else if (status.MPI_TAG == WAIT)
            {
                // Fick inte besticken → vänta lite och försök igen
                printf("Filosof %d får vänta. Försöker igen om %d sek...\n",
                       phil_id, wait_time);
                fflush(stdout);
                sleep(wait_time);
            }
            // Sedan börjar loopen om (tänker igen etc.)
        }
    }
    else // annars server
    {
        for (int i = 0; i < NUM_PHILOSOPHERS; i++)
        {
            chopsticks[i] = -1; // Gör alla chopsticks lediga
        }

        printf("Serven startar på process %d.\n", world_rank);

        while (1)
        {
            MPI_Status status;
            MPI_Recv(NULL, 0, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &status);

            int phil = status.MPI_SOURCE;
            int tag = status.MPI_TAG;

            if (tag == PICKUP)
            {
                // Kolla om båda chopsticks är lediga
                int Left = left_chopstick(phil);
                int Right = right_chopstick(phil);

                if (chopsticks[Left] == -1 && chopsticks[Right] == -1)
                {
                    // Båda är lediga så då bevilja
                    chopsticks[Left] = phil;
                    chopsticks[Right] = phil;
                    printf("Serven: Beviljar besticken (%d,%d) till filosof %d.\n", Left, Right, phil);
                    fflush(stdout);
                    MPI_Send(NULL, 0, MPI_CHAR, phil, GRANT, MPI_COMM_WORLD);
                }
                else
                {
                    // Minst en är upptagen WAIT
                    MPI_Send(NULL, 0, MPI_CHAR, phil, WAIT, MPI_COMM_WORLD);
                }
            }
            else if (tag == PUTDOWN)
            {
                // Frigör filosofens chopsticks
                int Left = left_chopstick(phil);
                int Right = right_chopstick(phil);
                chopsticks[Left] = -1;
                chopsticks[Right] = -1;
                printf("Servern: Filosof %d lämnade tillbaka (%d,%d). Lediga nu.\n", phil, Left, Right);
                fflush(stdout);
            }
        }
    }

    MPI_Finalize();
    return 0;
}
