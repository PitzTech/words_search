#ifndef FILE_IO_H
#define FILE_IO_H

#include "types.h"
#include "grid.h"

Grid* readPuzzleFromFile(void);
void readWordsFromFile(char words[][MAX_WORD_LENGTH], int* numWords);

#endif // FILE_IO_H
