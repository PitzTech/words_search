#include "grid.h"
#include "types.h"
#include "debug.h"
#include "constants.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

Grid* Grid_create(int rows, int cols) {
    Grid* grid = (Grid*)malloc(sizeof(Grid));
    if (!grid) return NULL;

    grid->rows = rows;
    grid->cols = cols;

    // Allocate arrays
    grid->letters = (char**)malloc(rows * sizeof(char*));
    grid->highlighted = (char**)malloc(rows * sizeof(char*));

    if (!grid->letters || !grid->highlighted) {
        Grid_destroy(grid);
        return NULL;
    }

    // Initialize arrays
    for (int i = 0; i < rows; i++) {
        grid->letters[i] = (char*)malloc(cols * sizeof(char));
        grid->highlighted[i] = (char*)malloc(cols * sizeof(char));

        if (!grid->letters[i] || !grid->highlighted[i]) {
            Grid_destroy(grid);
            return NULL;
        }

        // Initialize highlighted array with spaces
        for (int j = 0; j < cols; j++) {
            grid->highlighted[i][j] = ' ';
        }
        debugPrint("DEBUG Create: Row %d highlighted array initialized\n", i);
    }

    return grid;
}

void Grid_destroy(Grid* grid) {
    if (!grid) return;

    if (grid->letters) {
        for (int i = 0; i < grid->rows; i++) {
            free(grid->letters[i]);
        }
        free(grid->letters);
    }

    if (grid->highlighted) {
        for (int i = 0; i < grid->rows; i++) {
            free(grid->highlighted[i]);
        }
        free(grid->highlighted);
    }

    free(grid);
}

bool Grid_isValidPosition(const Grid* grid, int row, int col) {
    return row >= 0 && row < grid->rows && col >= 0 && col < grid->cols;
}

void Grid_highlightWord(Grid* grid, const WordPosition pos) {
    debugPrint("\nDEBUG Highlight: Word '%s' from (%d,%d) to (%d,%d)\n",
           pos.word, pos.startRow, pos.startCol, pos.endRow, pos.endCol);

    int len = strlen(pos.word);
    if (len == 0) return;

    // Calculate direction vectors
    int rowDiff = pos.endRow - pos.startRow;
    int colDiff = pos.endCol - pos.startCol;

    // Handle wrapping for shortest path
    if (abs(rowDiff) > grid->rows/2) {
        rowDiff = rowDiff > 0 ? rowDiff - grid->rows : rowDiff + grid->rows;
    }
    if (abs(colDiff) > grid->cols/2) {
        colDiff = colDiff > 0 ? colDiff - grid->cols : colDiff + grid->cols;
    }

    // Calculate step direction
    int rowStep = 0;
    int colStep = 0;

    if (rowDiff != 0) {
        rowStep = rowDiff / abs(rowDiff);
    }
    if (colDiff != 0) {
        colStep = colDiff / abs(colDiff);
    }

    debugPrint("DEBUG Highlight: Direction steps - row: %d, col: %d\n", rowStep, colStep);

    // Highlight each letter
    int currentRow = pos.startRow;
    int currentCol = pos.startCol;

    for (int i = 0; i < len; i++) {
        // Handle wrapping
        while (currentRow < 0) currentRow += grid->rows;
        while (currentRow >= grid->rows) currentRow -= grid->rows;
        while (currentCol < 0) currentCol += grid->cols;
        while (currentCol >= grid->cols) currentCol -= grid->cols;

        debugPrint("DEBUG Highlight: Marking position (%d,%d) with letter '%c'\n",
               currentRow, currentCol, grid->letters[currentRow][currentCol]);

        // Set highlight
        grid->highlighted[currentRow][currentCol] = grid->letters[currentRow][currentCol];

        // Verify highlight
        debugPrint("DEBUG Highlight: After marking - highlighted value is '%c'\n",
               grid->highlighted[currentRow][currentCol]);

        // Move to next position
        currentRow += rowStep;
        currentCol += colStep;
    }

    debugPrint("DEBUG Highlight: Finished highlighting word '%s'\n", pos.word);
}

void Grid_print(const Grid* grid) {
    if (!grid) return;

    // Print column numbers
    printf("    ");
    for (int j = 0; j < grid->cols; j++) {
        printf("%2d ", j);
    }
    printf("\n");

    // Print grid with row numbers and highlights
    for (int i = 0; i < grid->rows; i++) {
        printf("%2d  ", i);
        for (int j = 0; j < grid->cols; j++) {
            // Get current cell state
            char letterChar = grid->letters[i][j];
            char highlightChar = grid->highlighted[i][j];

            debugPrint("DEBUG Print: Position (%d,%d) - letter='%c', highlighted='%c'\n",
                   i, j, letterChar, highlightChar);

            // Print with or without highlight
            if (highlightChar != ' ') {
                printf("%s%c%s  ", COLORS.green, letterChar, COLORS.reset);
            } else {
                printf("%c  ", letterChar);
            }
        }
        printf("\n");
    }
}
