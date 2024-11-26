#ifndef MPI_HANDLER_H
#define MPI_HANDLER_H

#include "types.h"
#include "grid.h"
#include <mpi.h>

void broadcastGridData(Grid* grid, int numWords, char words[][MAX_WORD_LENGTH]);
RowRange calculateWorkDistribution(int rank, int size, int totalRows);
void handleMasterProcess(int rank, int size, OutputOptions* options);
void handleWorkerProcess(int rank, int size);
void syncHighlightedArrays(Grid* grid, ProcessResults* allResults, int size);

#endif // MPI_HANDLER_H
