#ifndef DEBUG_H
#define DEBUG_H

#include "types.h"
#include "grid.h"

void debugPrint(const char* format, ...);
void debugSearchAttempt(const Grid* grid, int row, int col, Direction dir, const char* word);

#endif // DEBUG_H
