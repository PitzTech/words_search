#include "mpi_handler.h"
#include "file_io.h"
#include "search.h"
#include "output.h"
#include "debug.h"
#include "constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void syncHighlightedArrays(Grid* grid, ProcessResults* allResults, int size) {
    if (grid == NULL) return;

    // Process each result
    for (int proc = 0; proc < size; proc++) {
        for (int i = 0; i < allResults[proc].validResults; i++) {
            WordPosition pos = allResults[proc].positions[i];
            debugPrint("DEBUG Sync: Processing word '%s' from process %d\n", pos.word, proc);
            Grid_highlightWord(grid, pos);
        }
    }
}

void handleMasterProcess(int rank, int size, OutputOptions* options) {
    double startTime = MPI_Wtime();

    // Initialize master process and read input
    char words[MAX_WORDS][MAX_WORD_LENGTH];
    int numWords;
    Grid* grid = readPuzzleFromFile();

    if (!grid) {
        fprintf(stderr, "Error: Failed to read puzzle\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    readWordsFromFile(words, &numWords);

    // Print initial information
    printf("\nPuzzle Information:\n");
    printf("------------------\n");
    printf("Grid dimensions: %d columns x %d rows\n", grid->cols, grid->rows);
    printf("Number of words to search: %d\n", numWords);
    printf("Words to find: ");
    for (int i = 0; i < numWords; i++) {
        printf("%s", words[i]);
        if (i < numWords - 1) printf(", ");
    }
    printf("\n\n");

    // Broadcast data to all processes
    broadcastGridData(grid, numWords, words);

    // Calculate work distribution
    RowRange range = calculateWorkDistribution(rank, size, grid->rows);

    // Search words in master's portion
    ProcessResults myResults = searchWords(grid, words, numWords, range);

    // Gather all results
    ProcessResults* allResults = (ProcessResults*)malloc(size * sizeof(ProcessResults));
    if (!allResults) {
        fprintf(stderr, "Error: Failed to allocate memory for results\n");
        Grid_destroy(grid);
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    MPI_Gather(&myResults, sizeof(ProcessResults), MPI_BYTE,
               allResults, sizeof(ProcessResults), MPI_BYTE,
               0, MPI_COMM_WORLD);

    // Synchronize highlighted arrays
    syncHighlightedArrays(grid, allResults, size);

    // Process and display results
    printf("\nSearch Results:\n");
    printf("--------------\n");
    Grid_print(grid);

    // Export results if output file is specified
    if (options && options->outputFile) {
        Grid_exportToFile(grid, options->outputFile, options->useHTML);
    }

    printf("\nFound Words:\n");
    int totalFound = 0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < allResults[i].validResults; j++) {
            printFoundWord(&allResults[i].positions[j]);
            totalFound++;
        }
    }

    // Print execution time
    double endTime = MPI_Wtime();
    printPerformanceMetrics(totalFound, startTime, endTime, size);

    // Cleanup
    free(allResults);
    Grid_destroy(grid);
}

void handleWorkerProcess(int rank, int size) {
    // Receive grid dimensions and data
    int rows, cols, numWords;
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&numWords, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Create grid
    Grid* grid = Grid_create(rows, cols);
    if (!grid) {
        fprintf(stderr, "Error: Failed to create grid in worker process %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    // Receive grid data and words
    char words[MAX_WORDS][MAX_WORD_LENGTH];
    for (int i = 0; i < rows; i++) {
        MPI_Bcast(grid->letters[i], cols, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    for (int i = 0; i < numWords; i++) {
        MPI_Bcast(words[i], MAX_WORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    // Calculate work distribution
    RowRange range = calculateWorkDistribution(rank, size, rows);

    // Search words in worker's portion
    ProcessResults myResults = searchWords(grid, words, numWords, range);

    // Send results back to master
    MPI_Gather(&myResults, sizeof(ProcessResults), MPI_BYTE,
               NULL, sizeof(ProcessResults), MPI_BYTE,
               0, MPI_COMM_WORLD);

    // Cleanup
    Grid_destroy(grid);
}

void broadcastGridData(Grid* grid, int numWords, char words[][MAX_WORD_LENGTH]) {
    MPI_Bcast(&grid->rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&grid->cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&numWords, 1, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < grid->rows; i++) {
        MPI_Bcast(grid->letters[i], grid->cols, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    for (int i = 0; i < numWords; i++) {
        MPI_Bcast(words[i], MAX_WORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
    }
}

RowRange calculateWorkDistribution(int rank, int size, int totalRows) {
    RowRange range;
    int baseRows = totalRows / size;
    int extraRows = totalRows % size;

    range.start = rank * baseRows + (rank < extraRows ? rank : extraRows);
    range.end = range.start + baseRows + (rank < extraRows ? 1 : 0);

    debugPrint("DEBUG: Process %d assigned rows %d to %d\n", rank, range.start, range.end - 1);

    return range;
}
