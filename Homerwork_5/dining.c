#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // För sleep

// Definiera meddelande tag
#define PICKUP 0
#define PUTDOWN 1

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Vi vill ha 10 processer: 5 filosofer och 5 chopsticks
    // Varje filosof behöver 2 chopsticks för att öta
    // Fairness är med i beräkning

    // Programmet ska köras med 10 processer
    if (world_size != 10)
    {
        if (world_rank == 0)
        {
            printf("Programmet ska köras med 10 processer: 5 filosofer och 5 chopsticks.\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    // Filosofprocess arbetetddd
    int filosof_id = world_rank / 2; // Filosofernas ID: 0 till 4

    // Bestäm vänster och höger chopsticks.
    // Vänster chopsticks: (rank + size - 1) % size
    // Höger chopsticks: (rank + 1) % size
    int left_chopstick = (world_rank + world_size - 1) % world_size;
    int righ_chopstick = (world_rank + 1) % world_size;

    // För att undvika deadlock låter vi filosof 0 plocka i omvänd ordning.
    int pick_left_first = (filosof_id != 0);

    if (world_rank % 2 == 0) // Filosofer är processer med jämn rank.
    {
        while (1)
        {
            // Tänka
            printf("Filosof %d tänker...\n", filosof_id);
            sleep(1);

            // Plocka upp chopsticks
            if (pick_left_first)
            {
                // Plocka vänster chopsticks först
                printf("Filosof %d ber om att få plocka upp vänster chopsticks (process %d)...\n", filosof_id, left_chopstick);
                MPI_Send(NULL, 0, MPI_CHAR, left_chopstick, PICKUP, MPI_COMM_WORLD);
                // Sedan höger chopsticks
                printf("Filosof %d ber om att få plocka upp höger chopsticks (process %d)...\n", filosof_id, righ_chopstick);
                MPI_Send(NULL, 0, MPI_CHAR, righ_chopstick, PICKUP, MPI_COMM_WORLD);
            }
            else // För att optimera och undivika deadlock så plockar filosof 0 upp höger chopstick först
            {
                // Filosof 0 plockar höger chopsticks först
                printf("Filosof %d ber om att få plocka upp höger chopsticks (process %d)...\n", filosof_id, righ_chopstick);
                MPI_Send(NULL, 0, MPI_CHAR, righ_chopstick, PICKUP, MPI_COMM_WORLD);
                printf("Filosof %d ber om att få plocka upp vänster chopsticks (process %d)...\n", filosof_id, left_chopstick);
                MPI_Send(NULL, 0, MPI_CHAR, left_chopstick, PICKUP, MPI_COMM_WORLD);
            }

            // Äta
            printf("Filosof %d äter...\n", filosof_id);
            sleep(1);

            // Lägg ner chopsticks
            if (pick_left_first)
            {
                printf("Filosof %d lägger tillbaka vänster chopsticks (process %d)...\n", filosof_id, left_chopstick);
                MPI_Send(NULL, 0, MPI_CHAR, left_chopstick, PUTDOWN, MPI_COMM_WORLD);
                printf("Filosof %d lägger tillbaka höger chopsticks (process %d)...\n", filosof_id, righ_chopstick);
                MPI_Send(NULL, 0, MPI_CHAR, righ_chopstick, PUTDOWN, MPI_COMM_WORLD);
            }
            else
            {
                printf("Filosof %d lägger tillbaka höger chopsticks (process %d)...\n", filosof_id, righ_chopstick);
                MPI_Send(NULL, 0, MPI_CHAR, righ_chopstick, PUTDOWN, MPI_COMM_WORLD);
                printf("Filosof %d lägger tillbaka vänster chopsticks (process %d)...\n", filosof_id, left_chopstick);
                MPI_Send(NULL, 0, MPI_CHAR, left_chopstick, PUTDOWN, MPI_COMM_WORLD);
            }
        }
    }
    else // Chopsticks är processer med udda rank.
    {
        // Varje chopsticks process hanterar begäranden från de intilliggande filosoferna.
        while (1)
        {
            MPI_Status status; // Väntar på status förändring, att någon filosof ska begära att plocka upp chopstickset
            MPI_Recv(NULL, 0, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); //När statusen förändras kan den ta emot sending med reciveing

            if (status.MPI_TAG == PICKUP) //Om statusen är PICKUP gör följande
            {
                int requester = status.MPI_SOURCE;
                printf("Chopsticks nummer %d har blivit upp plockad av filosof %d.\n", world_rank, requester);
                // Vänta på att samma filosof lägger ner chopstickset
                MPI_Recv(NULL, 0, MPI_CHAR, requester, PUTDOWN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Chopsticks nummer %d har blivit ner lagd av filosof %d.\n", world_rank, requester);
            }
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
