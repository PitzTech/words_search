#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>

// Constants
#define MAX_WORD_LENGTH 50
#define MAX_WORDS 100
#define MAX_LINE_LENGTH 2000
#define INITIAL_GRID_CAPACITY 10
#define DIRECTIONS_COUNT 8
#define MAX_RESULTS_PER_WORD (DIRECTIONS_COUNT)
#define MAX_TOTAL_RESULTS (MAX_WORDS * MAX_RESULTS_PER_WORD)
#define DEBUG 0

// Structures
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

typedef struct {
    int dx;
    int dy;
    const char* name;
} DirectionVector;

typedef struct {
    const char* green;
    const char* reset;
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

// Constants initialization
const ColorCodes COLORS = {
    .green = "\033[1;32m",
    .reset = "\033[0m"
};

const DirectionVector DIRECTION_VECTORS[DIRECTIONS_COUNT] = {
    {-1,  0, "UP"},
    { 1,  0, "DOWN"},
    { 0, -1, "LEFT"},
    { 0,  1, "RIGHT"},
    {-1, -1, "UP_LEFT"},
    {-1,  1, "UP_RIGHT"},
    { 1, -1, "DOWN_LEFT"},
    { 1,  1, "DOWN_RIGHT"}
};

// Debug Functions
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

// Function Prototypes
Grid* Grid_create(int rows, int cols);
void Grid_destroy(Grid* grid);
bool Grid_isValidPosition(const Grid* grid, int row, int col);
void Grid_highlightWord(Grid* grid, const WordPosition pos);
void Grid_print(const Grid* grid);
static int countTokens(const char* str);
Grid* readPuzzleFromFile(void);
void readWordsFromFile(char words[][MAX_WORD_LENGTH], int* numWords);
bool searchWordInDirection(const Grid* grid, int startRow, int startCol,
                         Direction dir, const char* word, WordPosition* pos);
void searchWordParallel(const Grid* grid, const char* word, int startRow,
                       int endRow, WordPosition* positions, int* count);
RowRange calculateWorkDistribution(int rank, int size, int totalRows);
ProcessResults searchWords(Grid* grid, const char words[][MAX_WORD_LENGTH],
                         int numWords, RowRange range);

// Grid Management Functions
Grid* Grid_create(int rows, int cols) {
    Grid* grid = (Grid*)malloc(sizeof(Grid));
    if (!grid) return NULL;

    grid->rows = rows;
    grid->cols = cols;

    grid->letters = (char**)malloc(rows * sizeof(char*));
    grid->highlighted = (char**)malloc(rows * sizeof(char*));

    if (!grid->letters || !grid->highlighted) {
        Grid_destroy(grid);
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        grid->letters[i] = (char*)malloc(cols * sizeof(char));
        grid->highlighted[i] = (char*)malloc(cols * sizeof(char));

        if (!grid->letters[i] || !grid->highlighted[i]) {
            Grid_destroy(grid);
            return NULL;
        }

        memset(grid->highlighted[i], ' ', cols);
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
    int len = strlen(pos.word);
    int row = pos.startRow;
    int col = pos.startCol;

    // Calculate direction vectors considering wrapping
    int drow = pos.endRow - pos.startRow;
    int dcol = pos.endCol - pos.startCol;

    if (drow != 0) {
        if (abs(drow) > grid->rows/2) {
            drow = (drow > 0) ? -1 : 1;
        } else {
            drow = (drow > 0) ? 1 : -1;
        }
    }

    if (dcol != 0) {
        if (abs(dcol) > grid->cols/2) {
            dcol = (dcol > 0) ? -1 : 1;
        } else {
            dcol = (dcol > 0) ? 1 : -1;
        }
    }

    // Highlight each letter
    for (int i = 0; i < len; i++) {
        row = (row + grid->rows) % grid->rows;
        col = (col + grid->cols) % grid->cols;
        grid->highlighted[row][col] = grid->letters[row][col];
        row += drow;
        col += dcol;
    }
}

void Grid_print(const Grid* grid) {
    printf("    ");
    for (int j = 0; j < grid->cols; j++) {
        printf("%2d ", j);
    }
    printf("\n");

    for (int i = 0; i < grid->rows; i++) {
        printf("%2d  ", i);
        for (int j = 0; j < grid->cols; j++) {
            if (grid->highlighted[i][j] != ' ') {
                printf("%s%c%s  ", COLORS.green, grid->highlighted[i][j], COLORS.reset);
            } else {
                printf("%c  ", grid->letters[i][j]);
            }
        }
        printf("\n");
    }
}

// Search Functions
bool searchWordInDirection(const Grid* grid, int startRow, int startCol,
                         Direction dir, const char* word, WordPosition* pos) {
    debugSearchAttempt(grid, startRow, startCol, dir, word);

    int len = strlen(word);
    if (len == 0) return false;

    DirectionVector vector = DIRECTION_VECTORS[dir];
    int row = startRow;
    int col = startCol;

    for (int i = 0; i < len; i++) {
        // Handle wrapping
        row = (row + grid->rows) % grid->rows;
        col = (col + grid->cols) % grid->cols;

        debugPrint("Checking position (%d,%d) for letter %c against %c\n",
                  row, col, word[i], grid->letters[row][col]);

        if (tolower(grid->letters[row][col]) != tolower(word[i])) {
            return false;
        }

        row += vector.dx;
        col += vector.dy;
    }

    pos->startRow = startRow;
    pos->startCol = startCol;
    pos->endRow = ((startRow + (len-1) * vector.dx) + grid->rows) % grid->rows;
    pos->endCol = ((startCol + (len-1) * vector.dy) + grid->cols) % grid->cols;
    strcpy(pos->word, word);

    debugPrint("Found word %s from (%d,%d) to (%d,%d)\n",
              word, pos->startRow, pos->startCol, pos->endRow, pos->endCol);

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

// Work Distribution
RowRange calculateWorkDistribution(int rank, int size, int totalRows) {
    RowRange range;
    int baseRows = totalRows / size;
    int extraRows = totalRows % size;

    range.start = rank * baseRows + (rank < extraRows ? rank : extraRows);
    range.end = range.start + baseRows + (rank < extraRows ? 1 : 0);

    debugPrint("Process %d assigned rows %d to %d\n", rank, range.start, range.end - 1);

    return range;
}

// File Reading Helper
static int countTokens(const char* str) {
    int count = 0;
    char* copy = strdup(str);
    if (!copy) return 0;

    char* token = strtok(copy, " \t\n");
    while (token != NULL) {
        if (strlen(token) > 0) count++;
        token = strtok(NULL, " \t\n");
    }

    free(copy);
    return count;
}

// File Reading Functions
Grid* readPuzzleFromFile(void) {
    char** lines = NULL;
    int capacity = INITIAL_GRID_CAPACITY;
    int numLines = 0;
    char line[MAX_LINE_LENGTH];
    int maxCols = 0;

    lines = (char**)malloc(capacity * sizeof(char*));
    if (!lines) return NULL;

    while (fgets(line, MAX_LINE_LENGTH, stdin)) {
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0 || strstr(line, "Palavras:") != NULL) {
            break;
        }

        if (numLines >= capacity) {
            capacity *= 2;
            char** temp = realloc(lines, capacity * sizeof(char*));
            if (!temp) {
                for (int i = 0; i < numLines; i++) {
                    free(lines[i]);
                }
                free(lines);
                return NULL;
            }
            lines = temp;
        }

        lines[numLines] = strdup(line);
        if (!lines[numLines]) {
            for (int i = 0; i < numLines; i++) {
                free(lines[i]);
            }
            free(lines);
            return NULL;
        }

        int cols = countTokens(line);
        if (cols > maxCols) maxCols = cols;

        numLines++;
    }

    Grid* grid = Grid_create(numLines, maxCols);
    if (!grid) {
        for (int i = 0; i < numLines; i++) {
            free(lines[i]);
        }
        free(lines);
        return NULL;
    }

    for (int i = 0; i < numLines; i++) {
        char* token = strtok(lines[i], " \t\n");
        int j = 0;
        while (token != NULL && j < maxCols) {
            grid->letters[i][j] = token[0];
            j++;
            token = strtok(NULL, " \t\n");
        }
        free(lines[i]);
    }
    free(lines);

    return grid;
}

void readWordsFromFile(char words[][MAX_WORD_LENGTH], int* numWords) {
    char line[MAX_LINE_LENGTH];
    *numWords = 0;

    while (fgets(line, MAX_LINE_LENGTH, stdin)) {
        if (strstr(line, "Palavras:") != NULL) {
            break;
        }
    }

    if (fgets(line, MAX_LINE_LENGTH, stdin)) {
        line[strcspn(line, "\n")] = 0;
        char* token = strtok(line, " ,");

        while (token != NULL && *numWords < MAX_WORDS) {
            while (isspace(*token)) token++;
            strcpy(words[*numWords], token);
            (*numWords)++;
            token = strtok(NULL, " ,");
        }
    }
}

// MPI Communication Functions
void broadcastGridData(Grid* grid, int numWords, char words[][MAX_WORD_LENGTH]) {
    // Broadcast dimensions
    MPI_Bcast(&grid->rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&grid->cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&numWords, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Broadcast grid data
    for (int i = 0; i < grid->rows; i++) {
        MPI_Bcast(grid->letters[i], grid->cols, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    // Broadcast words
    for (int i = 0; i < numWords; i++) {
        MPI_Bcast(words[i], MAX_WORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
    }
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

void processSearchResults(Grid* grid, ProcessResults* allResults, int size) {
    int totalFound = 0;

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < allResults[i].validResults; j++) {
            Grid_highlightWord(grid, allResults[i].positions[j]);
            totalFound++;
        }
    }

    debugPrint("Total words found across all processes: %d\n", totalFound);
}

void printResults(Grid* grid, ProcessResults* allResults, int size,
                 double startTime, double endTime) {
    printf("\nSearch Results:\n");
    printf("--------------\n");

    Grid_print(grid);

    printf("\nFound Words:\n");
    int totalFound = 0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < allResults[i].validResults; j++) {
            printFoundWord(&allResults[i].positions[j]);
            totalFound++;
        }
    }

    printPerformanceMetrics(totalFound, startTime, endTime, size);
}

void printFoundWord(const WordPosition* pos) {
    printf("%s: (%d,%d) to (%d,%d)\n",
           pos->word, pos->startRow, pos->startCol,
           pos->endRow, pos->endCol);
}

void printPerformanceMetrics(int totalFound, double startTime, double endTime,
                           int numProcesses) {
    double totalTime = endTime - startTime;
    double wordsPerSecond = totalFound / totalTime;

    printf("\nPerformance Metrics:\n");
    printf("-------------------\n");
    printf("Total words found: %d\n", totalFound);
    printf("Execution time: %.4f seconds\n", totalTime);
    printf("Processing speed: %.2f words/second\n", wordsPerSecond);
    printf("Number of processes: %d\n", numProcesses);
}

// Process Handler Functions
void handleMasterProcess(int rank, int size) {
    double startTime = MPI_Wtime();

    // Initialize master process and read input
    char words[MAX_WORDS][MAX_WORD_LENGTH];
    int numWords;
    Grid* grid = readPuzzleFromFile();

    if (!grid) {
        fprintf(stderr, "Error: Failed to read puzzle\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    readWordsFromFile(words, &numWords);

    // Print initial information
    printf("\nPuzzle Information:\n");
    printf("------------------\n");
    printf("Grid dimensions: %d columns x %d rows\n", grid->cols, grid->rows);
    printf("Number of words to search: %d\n", numWords);
    printf("Words to find: ");
    for (int i = 0; i < numWords; i++) {
        printf("%s", words[i]);
        if (i < numWords - 1) printf(", ");
    }
    printf("\n\n");

    // Broadcast data to all processes
    broadcastGridData(grid, numWords, words);

    // Calculate work distribution
    RowRange range = calculateWorkDistribution(rank, size, grid->rows);

    // Search words in master's portion
    ProcessResults myResults = searchWords(grid, words, numWords, range);

    // Gather all results
    ProcessResults* allResults = (ProcessResults*)malloc(size * sizeof(ProcessResults));
    if (!allResults) {
        fprintf(stderr, "Error: Failed to allocate memory for results\n");
        Grid_destroy(grid);
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    MPI_Gather(&myResults, sizeof(ProcessResults), MPI_BYTE,
               allResults, sizeof(ProcessResults), MPI_BYTE,
               0, MPI_COMM_WORLD);

    // Process and display results
    double endTime = MPI_Wtime();
    processSearchResults(grid, allResults, size);
    printResults(grid, allResults, size, startTime, endTime);

    // Cleanup
    free(allResults);
    Grid_destroy(grid);
}

void handleWorkerProcess(int rank, int size) {
    // Receive grid dimensions and data
    int rows, cols, numWords;
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&numWords, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Create grid
    Grid* grid = Grid_create(rows, cols);
    if (!grid) {
        fprintf(stderr, "Error: Failed to create grid in worker process %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    // Receive grid data
    char words[MAX_WORDS][MAX_WORD_LENGTH];
    for (int i = 0; i < rows; i++) {
        MPI_Bcast(grid->letters[i], cols, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    for (int i = 0; i < numWords; i++) {
        MPI_Bcast(words[i], MAX_WORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    // Calculate work distribution
    RowRange range = calculateWorkDistribution(rank, size, rows);

    // Search words in worker's portion
    ProcessResults myResults = searchWords(grid, words, numWords, range);

    // Send results back to master
    MPI_Gather(&myResults, sizeof(ProcessResults), MPI_BYTE,
               NULL, sizeof(ProcessResults), MPI_BYTE,
               0, MPI_COMM_WORLD);

    // Cleanup
    Grid_destroy(grid);
}

int main(int argc, char** argv) {
    int rank, size;

    // Initialize MPI
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        fprintf(stderr, "Error: Failed to initialize MPI\n");
        return 1;
    }

    // Get process rank and total size
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Validate number of processes
    if (size < 1) {
        if (rank == 0) {
            fprintf(stderr, "Error: At least one process is required\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Handle process based on rank
    if (rank == 0) {
        handleMasterProcess(rank, size);
    } else {
        handleWorkerProcess(rank, size);
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}
