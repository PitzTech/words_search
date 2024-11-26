#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#define MAX_WORD_LENGTH 50
#define MAX_WORDS 100
#define MAX_LINE_LENGTH 2000
#define INITIAL_GRID_CAPACITY 10
#define DIRECTIONS_COUNT 8
#define MAX_RESULTS_PER_WORD (DIRECTIONS_COUNT)
#define MAX_TOTAL_RESULTS (MAX_WORDS * MAX_RESULTS_PER_WORD)
#define DEBUG 0

// Enumeration for search directions
typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP_LEFT,
    DIR_UP_RIGHT,
    DIR_DOWN_LEFT,
    DIR_DOWN_RIGHT
} Direction;

// Basic structures
typedef struct {
    int dx;
    int dy;
    const char* name;
} DirectionVector;

typedef struct {
    const char* green;
    const char* reset;
    const char* html_green_start;
    const char* html_green_end;
} ColorCodes;

typedef struct {
    int startRow;
    int startCol;
    int endRow;
    int endCol;
    char word[MAX_WORD_LENGTH];
} WordPosition;

typedef struct {
    int validResults;
    int totalProcessed;
    WordPosition positions[MAX_TOTAL_RESULTS];
} ProcessResults;

typedef struct {
    char** letters;
    char** highlighted;
    int rows;
    int cols;
} Grid;

typedef struct {
    int start;
    int end;
} RowRange;

typedef struct {
    char* outputFile;   // Output file path
    bool useHTML;       // HTML output flag
} OutputOptions;

#endif // TYPES_H
