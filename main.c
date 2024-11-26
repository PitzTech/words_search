#include "mpi_handler.h"
#include "debug.h"
#include <stdio.h>

int main(int argc, char** argv) {
    int rank, size;

    // Initialize MPI
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        fprintf(stderr, "Error: Failed to initialize MPI\n");
        return 1;
    }

    // Get process rank and total size
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Validate number of processes
    if (size < 1) {
        if (rank == 0) {
            fprintf(stderr, "Error: At least one process is required\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Handle process based on rank
    if (rank == 0) {
        handleMasterProcess(rank, size);
    } else {
        handleWorkerProcess(rank, size);
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}
