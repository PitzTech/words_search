#ifndef OUTPUT_H
#define OUTPUT_H

#include "types.h"
#include "grid.h"

void printResults(Grid* grid, ProcessResults* allResults, int size,
                 double startTime, double endTime);
void printFoundWord(const WordPosition* pos);
void printPerformanceMetrics(int totalFound, double startTime, double endTime,
                           int numProcesses);

#endif // OUTPUT_H
