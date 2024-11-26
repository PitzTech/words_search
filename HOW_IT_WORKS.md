# Word Search Puzzle Solver - Comprehensive Documentation

## Table of Contents
1. [Overview](#overview)
2. [Core Architecture](#core-architecture)
3. [Search Algorithm](#search-algorithm)
4. [Parallelization Strategy](#parallelization-strategy)
5. [Implementation Details](#implementation-details)
6. [Technical Specifications](#technical-specifications)
7. [Performance Considerations](#performance-considerations)

## Overview
This project implements a parallel word search puzzle solver using MPI (Message Passing Interface) for distributed processing. The program finds words in any direction (horizontal, vertical, diagonal) with wrap-around capabilities, meaning words can start at one edge of the grid and continue from the opposite edge.

## Core Architecture

### Components
1. **Grid Management** (`grid.h`, `grid.c`)
   ```c
   typedef struct {
       char** letters;      // The actual puzzle grid
       char** highlighted;  // Tracks found words for highlighting
       int rows;           // Grid dimensions
       int cols;
   } Grid;
   ```

2. **Word Position Tracking**
   ```c
   typedef struct {
       int startRow;
       int startCol;
       int endRow;
       int endCol;
       char word[MAX_WORD_LENGTH];
   } WordPosition;
   ```

3. **Process Results**
   ```c
   typedef struct {
       int validResults;
       int totalProcessed;
       WordPosition positions[MAX_TOTAL_RESULTS];
   } ProcessResults;
   ```

## Search Algorithm

### 1. Direction Vectors
```c
const DirectionVector DIRECTION_VECTORS[DIRECTIONS_COUNT] = {
    {-1,  0, "UP"},         // Moving up: decrease row
    { 1,  0, "DOWN"},       // Moving down: increase row
    { 0, -1, "LEFT"},       // Moving left: decrease column
    { 0,  1, "RIGHT"},      // Moving right: increase column
    {-1, -1, "UP_LEFT"},    // Moving diagonally: combine vectors
    {-1,  1, "UP_RIGHT"},
    { 1, -1, "DOWN_LEFT"},
    { 1,  1, "DOWN_RIGHT"}
};
```

### 2. Search Process
The search algorithm works by:
1. Examining each position in the grid as a potential starting point
2. For each position, trying all 8 possible directions
3. For each direction, checking if the word matches by:
   - Following the direction vector
   - Handling wrap-around at grid boundaries
   - Performing case-insensitive comparison
   - Recording successful finds

### 3. Wrap-Around Handling
```c
// Position normalization with wrap-around
row = (row + grid->rows) % grid->rows;
col = (col + grid->cols) % grid->cols;
```
This allows:
- Words to continue from bottom to top
- Words to continue from right to left
- Diagonal wrap-around
- Seamless grid connectivity

## Parallelization Strategy

### 1. Work Distribution
The grid is divided horizontally among available processors:
```
For a 30x30 grid with 4 processors:
┌─────────────┐
│ Processor 0 │ Rows 0-7
├─────────────┤
│ Processor 1 │ Rows 8-15
├─────────────┤
│ Processor 2 │ Rows 16-23
├─────────────┤
│ Processor 3 │ Rows 24-29
└─────────────┘
```

### 2. Process Roles

#### Master Process (Rank 0):
1. Reads input puzzle and word list
2. Broadcasts grid data and words to all processes
3. Processes its assigned section
4. Gathers results from all processes
5. Combines results and displays solution

#### Worker Processes (Rank > 0):
1. Receive grid and word data
2. Process assigned rows
3. Send results back to master

### 3. Communication Flow
```
Master                Workers
  │                     │
  ├─────Broadcast─────►│  Grid & Words
  │                     │
  │     Processing      │  Each process
  │                     │  searches its section
  │                     │
  ├◄────Gather────────│  Results
  │                     │
  └─────────────────────┘
```

## Implementation Details

### 1. Search Implementation
```c
bool searchWordInDirection(const Grid* grid, int startRow, int startCol,
                         Direction dir, const char* word, WordPosition* pos) {
    int len = strlen(word);
    int row = startRow;
    int col = startCol;
    DirectionVector vector = DIRECTION_VECTORS[dir];

    // Check each letter
    for (int i = 0; i < len; i++) {
        row = (row + grid->rows) % grid->rows;
        col = (col + grid->cols) % grid->cols;

        if (tolower(grid->letters[row][col]) != tolower(word[i])) {
            return false;
        }

        row += vector.dx;
        col += vector.dy;
    }

    // Store position information
    pos->startRow = startRow;
    pos->startCol = startCol;
    pos->endRow = (startRow + (len-1) * vector.dx + grid->rows) % grid->rows;
    pos->endCol = (startCol + (len-1) * vector.dy + grid->cols) % grid->cols;
    strcpy(pos->word, word);

    return true;
}
```

### 2. Work Distribution Implementation
```c
RowRange calculateWorkDistribution(int rank, int size, int totalRows) {
    int baseRows = totalRows / size;
    int extraRows = totalRows % size;

    RowRange range;
    range.start = rank * baseRows + (rank < extraRows ? rank : extraRows);
    range.end = range.start + baseRows + (rank < extraRows ? 1 : 0);

    return range;
}
```

## Technical Specifications

### Memory Management
- Dynamic allocation for grid structures
- Proper cleanup in all code paths
- Error handling for allocation failures
- Process-specific memory management

### Error Handling
- Input validation
- Memory allocation checks
- MPI communication verification
- Process coordination validation

### Performance Optimization
- Minimized inter-process communication
- Efficient memory access patterns
- Load balancing consideration
- Cache-friendly data structures

## Performance Considerations

### Scaling
- Performance improves with additional processors
- Optimal process count depends on grid size
- Communication overhead vs. processing gain
- Memory usage per process

### Load Balancing
- Even distribution of rows
- Handling of remainder rows
- Consideration of word density
- Process synchronization overhead

### Communication Overhead
- Initial data distribution
- Result collection
- Process synchronization
- Memory transfer costs

This system provides:
- Efficient parallel word searching
- Scalable performance with multiple processors
- Robust error handling
- Complete word finding capabilities
- Clear result presentation
