#ifndef GRID_H
#define GRID_H

#include "types.h"
#include <stdbool.h>

// Grid management functions
Grid* Grid_create(int rows, int cols);
void Grid_destroy(Grid* grid);
bool Grid_isValidPosition(const Grid* grid, int row, int col);
void Grid_highlightWord(Grid* grid, const WordPosition pos);
void Grid_print(const Grid* grid);

#endif // GRID_H
