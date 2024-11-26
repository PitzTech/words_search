#include "output.h"
#include "debug.h"
#include <stdio.h>

void printResults(Grid* grid, ProcessResults* allResults, int size,
                 double startTime, double endTime) {
    printf("\nSearch Results:\n");
    printf("--------------\n");

    Grid_print(grid);

    printf("\nFound Words:\n");
    int totalFound = 0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < allResults[i].validResults; j++) {
            printFoundWord(&allResults[i].positions[j]);
            totalFound++;
        }
    }

    printPerformanceMetrics(totalFound, startTime, endTime, size);
}

void printFoundWord(const WordPosition* pos) {
    printf("%s: (%d,%d) to (%d,%d)\n",
           pos->word, pos->startRow, pos->startCol,
           pos->endRow, pos->endCol);
}

void printPerformanceMetrics(int totalFound, double startTime, double endTime,
                           int numProcesses) {
    double totalTime = endTime - startTime;
    double wordsPerSecond = totalFound / totalTime;

    printf("\nPerformance Metrics:\n");
    printf("-------------------\n");
    printf("Total words found: %d\n", totalFound);
    printf("Execution time: %.4f seconds\n", totalTime);
    printf("Processing speed: %.2f words/second\n", wordsPerSecond);
    printf("Number of processes: %d\n", numProcesses);
}
