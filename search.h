#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"
#include "grid.h"

bool searchWordInDirection(const Grid* grid, int startRow, int startCol,
                         Direction dir, const char* word, WordPosition* pos);
void searchWordParallel(const Grid* grid, const char* word, int startRow,
                       int endRow, WordPosition* positions, int* count);
ProcessResults searchWords(Grid* grid, const char words[][MAX_WORD_LENGTH],
                         int numWords, RowRange range);

#endif // SEARCH_H
