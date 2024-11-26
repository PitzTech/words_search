#include "debug.h"
#include <stdarg.h>
#include <stdio.h>

void debugPrint(const char* format, ...) {
    #if DEBUG
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    #endif
}

void debugSearchAttempt(const Grid* grid, int row, int col, Direction dir, const char* word) {
    #if DEBUG
    printf("Searching for '%s' at (%d,%d) in direction %s\n",
           word, row, col, DIRECTION_VECTORS[dir].name);
    #endif
}
