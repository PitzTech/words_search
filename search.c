#include "search.h"
#include "debug.h"
#include "types.h"
#include "constants.h"
#include <string.h>
#include <ctype.h>

// extern const DirectionVector DIRECTION_VECTORS[DIRECTIONS_COUNT];

bool searchWordInDirection(const Grid* grid, int startRow, int startCol,
                         Direction dir, const char* word, WordPosition* pos) {
    debugPrint("DEBUG: Searching for word %s at (%d,%d) in direction %s\n",
           word, startRow, startCol, DIRECTION_VECTORS[dir].name);

    int len = strlen(word);
    if (len == 0) return false;

    // Initialize current position
    int currentRow = startRow;
    int currentCol = startCol;
    DirectionVector vector = DIRECTION_VECTORS[dir];

    // Check each letter
    for (int i = 0; i < len; i++) {
        // Handle wrapping
        currentRow = (currentRow + grid->rows) % grid->rows;
        currentCol = (currentCol + grid->cols) % grid->cols;

        char gridChar = tolower(grid->letters[currentRow][currentCol]);
        char wordChar = tolower(word[i]);

        debugPrint("DEBUG: Comparing grid[%d][%d]='%c' with word[%d]='%c'\n",
               currentRow, currentCol, gridChar, i, wordChar);

        if (gridChar != wordChar) {
            return false;
        }

        // Move to next position
        currentRow += vector.dx;
        currentCol += vector.dy;
    }

    // Word found - store positions
    pos->startRow = startRow;
    pos->startCol = startCol;
    pos->endRow = (startRow + (len-1) * vector.dx + grid->rows) % grid->rows;
    pos->endCol = (startCol + (len-1) * vector.dy + grid->cols) % grid->cols;
    strcpy(pos->word, word);

    debugPrint("DEBUG: Found word! Start=(%d,%d), End=(%d,%d)\n",
           pos->startRow, pos->startCol, pos->endRow, pos->endCol);

    return true;
}

void searchWordParallel(const Grid* grid, const char* word, int startRow, int endRow,
                       WordPosition* positions, int* count) {
    int localCount = *count;

    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < grid->cols; j++) {
            for (Direction dir = 0; dir < DIRECTIONS_COUNT; dir++) {
                WordPosition pos;
                if (searchWordInDirection(grid, i, j, dir, word, &pos)) {
                    positions[localCount] = pos;
                    localCount++;
                }
            }
        }
    }

    *count = localCount;
}

ProcessResults searchWords(Grid* grid, const char words[][MAX_WORD_LENGTH],
                         int numWords, RowRange range) {
    ProcessResults results = {0};

    for (int w = 0; w < numWords; w++) {
        int foundBefore = results.validResults;
        int newFound = 0;

        searchWordParallel(grid, words[w], range.start, range.end,
                          results.positions + results.validResults,
                          &newFound);

        results.validResults += newFound;
        results.totalProcessed++;

        debugPrint("Process found %d instances of word '%s'\n",
                  newFound, words[w]);
    }

    return results;
}
