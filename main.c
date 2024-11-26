#include "mpi_handler.h"
#include "debug.h"
#include <stdio.h>

void printUsage(const char* programName) {
    printf("Usage: %s [options]\n", programName);
    printf("Options:\n");
    printf("  -o, --output <file>    Output results to file\n");
    printf("  --html                 Output in HTML format\n");
    printf("  -h, --help            Show this help message\n");
}

int main(int argc, char** argv) {
    int rank, size;
    OutputOptions options = {NULL, false};  // Initialize with defaults

    // Process command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                options.outputFile = argv[++i];
                printf("Output will be written to: %s\n", options.outputFile);
            }
        } else if (strcmp(argv[i], "--html") == 0) {
            options.useHTML = true;
            printf("Using HTML format\n");
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        }
    }

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
        handleMasterProcess(rank, size, &options);
    } else {
        handleWorkerProcess(rank, size);
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}

