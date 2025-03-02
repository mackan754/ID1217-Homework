// mpicc distributedPairing1.c
//mpiexec -np 2 ./a.out

/*
This MPI program assigns student partners using a teacher process (rank 0). 
Each student (rank > 0) sends their rank to the teacher, who collects all requests in an array. 
The teacher then pairs students by swapping adjacent elements in the array. 
If there is an odd number of students, the last one remains unpaired. 
The teacher sends each student their assigned partner, and students print their pairings. 
MPI handles communication between processes, ensuring proper data exchange.
*/

#include <mpi.h>
#include <stdio.h>

#define TEACHER 0  

int main(int argc, char** argv) {
    int rank, size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == TEACHER) {
        int pairs[size - 1];

        // Collect student requests
        for (int i = 1; i < size; i++) {
            MPI_Recv(&pairs[i - 1], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // Pair students
        for (int i = 1; i < size - 1; i += 2) {
            int temp = pairs[i];
            pairs[i] = pairs[i - 1];
            pairs[i - 1] = temp;
        }

        // Send assigned partners
        for (int i = 1; i < size; i++) {
            MPI_Send(&pairs[i - 1], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Send(&rank, 1, MPI_INT, TEACHER, 0, MPI_COMM_WORLD);
        int partner;
        MPI_Recv(&partner, 1, MPI_INT, TEACHER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Student %d is paired with Student %d\n", rank, partner);
    }

    MPI_Finalize();
    return 0;
}
