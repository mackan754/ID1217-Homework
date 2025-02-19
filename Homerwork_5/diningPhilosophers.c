#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int message = rank; // Varje process startar med sitt rank-nummer

    int next = (rank + 1) % size; // Nästa process i ringen
    int prev = (rank - 1 + size) % size; // Föregående process i ringen

    MPI_Send(&message, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
    MPI_Recv(&message, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf("Process %d tog emot %d från process %d\n", rank, message, prev);

    MPI_Finalize();
    return 0;
}
