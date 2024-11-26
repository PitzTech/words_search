#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mpi.h>
#include <stdbool.h>

// Constants
#define MAX_WORD_LENGTH 50
#define MAX_WORDS 100
#define MAX_LINE_LENGTH 2000
#define INITIAL_GRID_CAPACITY 10

// Direction enumeration
typedef enum {
    DIR_UP,           // -1,  0
    DIR_DOWN,         //  1,  0
    DIR_LEFT,         //  0, -1
    DIR_RIGHT,        //  0,  1
    DIR_UP_LEFT,      // -1, -1
    DIR_UP_RIGHT,     // -1,  1
    DIR_DOWN_LEFT,    //  1, -1
    DIR_DOWN_RIGHT,   //  1,  1
    DIR_COUNT
} Direction;

// Direction vectors corresponding to Direction enum
typedef struct {
    int dx;
    int dy;
} DirectionVector;

const DirectionVector DIRECTION_VECTORS[] = {
    {-1,  0},  // UP
    { 1,  0},  // DOWN
    { 0, -1},  // LEFT
    { 0,  1},  // RIGHT
    {-1, -1},  // UP_LEFT
    {-1,  1},  // UP_RIGHT
    { 1, -1},  // DOWN_LEFT
    { 1,  1}   // DOWN_RIGHT
};

// Color codes for output
typedef struct {
    const char* green;
    const char* reset;
} ColorCodes;

const ColorCodes COLORS = {
    .green = "\033[1;32m",
    .reset = "\033[0m"
};

// Word position structure
typedef struct {
    int startRow;
    int startCol;
    int endRow;
    int endCol;
    char word[MAX_WORD_LENGTH];
} WordPosition;

// Grid structure
typedef struct {
    char** letters;       // The puzzle grid
    char** highlighted;   // Highlights for found words
    int rows;            // Number of rows
    int cols;            // Number of columns
} Grid;

// Function prototypes
Grid* Grid_create(int rows, int cols);
void Grid_destroy(Grid* grid);
bool Grid_isValidPosition(const Grid* grid, int row, int col);
void Grid_highlightWord(Grid* grid, const WordPosition pos);
void Grid_print(const Grid* grid);

WordPosition* WordPosition_create(int startRow, int startCol, int endRow, int endCol, const char* word);
Grid* readPuzzleFromFile(void);
void readWordsFromFile(char words[][MAX_WORD_LENGTH], int* numWords);
bool searchWordInDirection(const Grid* grid, int startRow, int startCol, Direction dir, const char* word, WordPosition* pos);
void searchWordParallel(const Grid* grid, const char* word, int startRow, int endRow, WordPosition* positions, int* count);
static int countTokens(const char* str);

// Grid functions implementation
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
    int row = pos.startRow;
    int col = pos.startCol;
    int len = strlen(pos.word);

    // Calculate direction
    int drow = (pos.endRow - pos.startRow + grid->rows) % grid->rows;
    int dcol = (pos.endCol - pos.startCol + grid->cols) % grid->cols;

    if (drow != 0) drow = drow > grid->rows/2 ? -1 : 1;
    if (dcol != 0) dcol = dcol > grid->cols/2 ? -1 : 1;

    for (int i = 0; i < len; i++) {
        grid->highlighted[row][col] = grid->letters[row][col];
        row = (row + drow + grid->rows) % grid->rows;
        col = (col + dcol + grid->cols) % grid->cols;
    }
}

void Grid_print(const Grid* grid) {
    printf("   ");
    for (int j = 0; j < grid->cols; j++) {
        printf("%2d ", j);
    }
    printf("\n");

    for (int i = 0; i < grid->rows; i++) {
        printf("%2d ", i);
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

bool searchWordInDirection(const Grid* grid, int startRow, int startCol,
                         Direction dir, const char* word, WordPosition* pos) {
    int len = strlen(word);
    int row = startRow;
    int col = startCol;
    DirectionVector vector = DIRECTION_VECTORS[dir];

    for (int i = 0; i < len; i++) {
        if (!Grid_isValidPosition(grid, row, col)) {
            row = (row + grid->rows) % grid->rows;
            col = (col + grid->cols) % grid->cols;
        }

        if (tolower(grid->letters[row][col]) != tolower(word[i])) {
            return false;
        }

        row += vector.dx;
        col += vector.dy;
    }

    pos->startRow = startRow;
    pos->startCol = startCol;
    pos->endRow = (startRow + (len-1) * vector.dx + grid->rows) % grid->rows;
    pos->endCol = (startCol + (len-1) * vector.dy + grid->cols) % grid->cols;
    strcpy(pos->word, word);

    return true;
}

void searchWordParallel(const Grid* grid, const char* word, int startRow, int endRow,
                       WordPosition* positions, int* count) {
    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < grid->cols; j++) {
            for (Direction dir = 0; dir < DIR_COUNT; dir++) {
                WordPosition pos;
                if (searchWordInDirection(grid, i, j, dir, word, &pos)) {
                    positions[*count] = pos;
                    (*count)++;
                }
            }
        }
    }
}

static int countTokens(const char* str) {
    int count = 0;
    char* copy = strdup(str);
    char* token = strtok(copy, " \t\n");

    while (token != NULL) {
        if (strlen(token) > 0) count++;
        token = strtok(NULL, " \t\n");
    }

    free(copy);
    return count;
}

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

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        // Master process reads input
        Grid* grid = readPuzzleFromFile();
        if (!grid) {
            fprintf(stderr, "Failed to read puzzle\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }

        char words[MAX_WORDS][MAX_WORD_LENGTH];
        int numWords;
        readWordsFromFile(words, &numWords);

        // Print the grid information
        printf("\nYour grid dimensions are %d columns x %d rows\n", grid->cols, grid->rows);
        printf("The number of words to search is: %d\n", numWords);
        printf("The words are: ");
        for (int i = 0; i < numWords; i++) {
            printf("%s", words[i]);
            if (i < numWords - 1) {
                printf(", ");
            }
        }
        printf("\n\n");

        // Broadcast data to all processes
        MPI_Bcast(&grid->rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&grid->cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
        for (int i = 0; i < grid->rows; i++) {
            MPI_Bcast(grid->letters[i], grid->cols, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
        MPI_Bcast(&numWords, 1, MPI_INT, 0, MPI_COMM_WORLD);
        for (int i = 0; i < numWords; i++) {
            MPI_Bcast(words[i], MAX_WORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
        }

        // Process work division
        int rowsPerProcess = grid->rows / size;
        WordPosition* allPositions = (WordPosition*)malloc(MAX_WORDS * size * sizeof(WordPosition));
        int totalFound = 0;

        // Search words in this process's portion
        for (int w = 0; w < numWords; w++) {
            int count = 0;
            searchWordParallel(grid, words[w], rank * rowsPerProcess,
                             (rank + 1) * rowsPerProcess, allPositions + totalFound, &count);
            totalFound += count;
        }

        // Gather results from all processes
        int* counts = (int*)malloc(size * sizeof(int));
        MPI_Gather(&totalFound, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Process and display results
        printf("\nWord Search Results:\n");

        // Highlight found words
        for (int i = 0; i < totalFound; i++) {
            Grid_highlightWord(grid, allPositions[i]);
        }

        // Print grid
        Grid_print(grid);

        // Print found words and their positions
        printf("\nFound words:\n");
        for (int i = 0; i < totalFound; i++) {
            printf("%s: (%d,%d) to (%d,%d)\n",
                   allPositions[i].word,
                   allPositions[i].startRow,
                   allPositions[i].startCol,
                   allPositions[i].endRow,
                   allPositions[i].endCol);
        }

        // Cleanup
        free(allPositions);
        free(counts);
        Grid_destroy(grid);
} else {
        // Worker processes
        int rows, cols, numWords;
        MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

        Grid* grid = Grid_create(rows, cols);
        if (!grid) {
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }

        // Receive grid data
        for (int i = 0; i < rows; i++) {
            MPI_Bcast(grid->letters[i], cols, MPI_CHAR, 0, MPI_COMM_WORLD);
        }

        MPI_Bcast(&numWords, 1, MPI_INT, 0, MPI_COMM_WORLD);
        char words[MAX_WORDS][MAX_WORD_LENGTH];
        for (int i = 0; i < numWords; i++) {
            MPI_Bcast(words[i], MAX_WORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
        }

        // Process this worker's portion
        int rowsPerProcess = rows / size;
        WordPosition* positions = (WordPosition*)malloc(MAX_WORDS * sizeof(WordPosition));
        int totalFound = 0;

        for (int w = 0; w < numWords; w++) {
            int count = 0;
            searchWordParallel(grid, words[w], rank * rowsPerProcess,
                             (rank + 1) * rowsPerProcess, positions + totalFound, &count);
            totalFound += count;
        }

        // Send results back to master
        MPI_Gather(&totalFound, 1, MPI_INT, NULL, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Cleanup
        free(positions);
        Grid_destroy(grid);
    }

    MPI_Finalize();
    return 0;
}
