#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mpi.h>
#include <stdbool.h>

#define MAX_WORD_LENGTH 50
#define MAX_WORDS 100
#define MAX_GRID_SIZE 1000
#define DIRECTIONS 8

// Structure to store word positions
typedef struct {
    int startRow;
    int startCol;
    int endRow;
    int endCol;
    char word[MAX_WORD_LENGTH];
} WordPosition;

// Structure to store the grid
typedef struct {
    char** letters;
    int rows;
    int cols;
    char** highlighted;  // For marking found words
} Grid;

// Direction vectors for all 8 possible directions
const int dx[] = {-1, -1, -1, 0, 1, 1, 1, 0};
const int dy[] = {-1, 0, 1, 1, 1, 0, -1, -1};

// Function to initialize the grid
Grid* initializeGrid(int rows, int cols) {
    Grid* grid = (Grid*)malloc(sizeof(Grid));
    grid->rows = rows;
    grid->cols = cols;

    // Allocate memory for letters
    grid->letters = (char**)malloc(rows * sizeof(char*));
    grid->highlighted = (char**)malloc(rows * sizeof(char*));
    for (int i = 0; i < rows; i++) {
        grid->letters[i] = (char*)malloc(cols * sizeof(char));
        grid->highlighted[i] = (char*)malloc(cols * sizeof(char));
        memset(grid->highlighted[i], ' ', cols);
    }

    return grid;
}

// Function to free grid memory
void freeGrid(Grid* grid) {
    for (int i = 0; i < grid->rows; i++) {
        free(grid->letters[i]);
        free(grid->highlighted[i]);
    }
    free(grid->letters);
    free(grid->highlighted);
    free(grid);
}

// Function to check if a position is valid in the grid
bool isValid(int row, int col, int rows, int cols) {
    return row >= 0 && row < rows && col >= 0 && col < cols;
}

// Function to search for a word in a specific direction
bool searchWord(Grid* grid, int startRow, int startCol, int dir,
                const char* word, WordPosition* pos) {
    int len = strlen(word);
    int row = startRow;
    int col = startCol;

    // Check if the word would fit in this direction
    for (int i = 0; i < len; i++) {
        if (!isValid(row, col, grid->rows, grid->cols)) {
            // Handle wrap-around
            row = (row + grid->rows) % grid->rows;
            col = (col + grid->cols) % grid->cols;
        }

        if (tolower(grid->letters[row][col]) != tolower(word[i])) {
            return false;
        }

        row += dx[dir];
        col += dy[dir];
    }

    // Word found - store position
    pos->startRow = startRow;
    pos->startCol = startCol;
    pos->endRow = (startRow + (len-1) * dx[dir] + grid->rows) % grid->rows;
    pos->endCol = (startCol + (len-1) * dy[dir] + grid->cols) % grid->cols;
    strcpy(pos->word, word);

    return true;
}

// Function to highlight a found word
void highlightWord(Grid* grid, WordPosition pos) {
    int row = pos.startRow;
    int col = pos.startCol;
    int len = strlen(pos.word);

    // Calculate direction
    int drow = (pos.endRow - pos.startRow + grid->rows) % grid->rows;
    int dcol = (pos.endCol - pos.startCol + grid->cols) % grid->cols;
    if (drow != 0) drow = drow > grid->rows/2 ? -1 : 1;
    if (dcol != 0) dcol = dcol > grid->cols/2 ? -1 : 1;

    // Mark each letter of the word
    for (int i = 0; i < len; i++) {
        grid->highlighted[row][col] = grid->letters[row][col];
        row = (row + drow + grid->rows) % grid->rows;
        col = (col + dcol + grid->cols) % grid->cols;
    }
}

// Main search function for a subset of the grid
void searchWordParallel(Grid* grid, char* word, int startRow, int endRow,
                       WordPosition* positions, int* count) {
    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < grid->cols; j++) {
            for (int dir = 0; dir < DIRECTIONS; dir++) {
                WordPosition pos;
                if (searchWord(grid, i, j, dir, word, &pos)) {
                    positions[*count] = pos;
                    (*count)++;
                }
            }
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
        int rows, cols;
        printf("Enter grid dimensions (rows cols): ");
        scanf("%d %d", &rows, &cols);

        Grid* grid = initializeGrid(rows, cols);

        printf("Enter the grid:\n");
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                scanf(" %c", &grid->letters[i][j]);
            }
        }

        int numWords;
        printf("Enter number of words to search: ");
        scanf("%d", &numWords);

        char words[MAX_WORDS][MAX_WORD_LENGTH];
        printf("Enter words:\n");
        for (int i = 0; i < numWords; i++) {
            scanf("%s", words[i]);
        }

        // Broadcast grid dimensions and data
        MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
        for (int i = 0; i < rows; i++) {
            MPI_Bcast(grid->letters[i], cols, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
        MPI_Bcast(&numWords, 1, MPI_INT, 0, MPI_COMM_WORLD);
        for (int i = 0; i < numWords; i++) {
            MPI_Bcast(words[i], MAX_WORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
        }

        // Divide work among processes
        int rowsPerProcess = rows / size;
        WordPosition* allPositions = (WordPosition*)malloc(MAX_WORDS * sizeof(WordPosition));
        int totalFound = 0;

        for (int w = 0; w < numWords; w++) {
            int count = 0;
            searchWordParallel(grid, words[w], rank * rowsPerProcess,
                             (rank + 1) * rowsPerProcess, allPositions + totalFound, &count);
            totalFound += count;
        }

        // Gather results
        int* counts = (int*)malloc(size * sizeof(int));
        MPI_Gather(&totalFound, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Highlight found words
        for (int i = 0; i < totalFound; i++) {
            highlightWord(grid, allPositions[i]);
        }

        // Print results
        printf("\nWord Search Results:\n");
        printf("   ");
        for (int j = 0; j < cols; j++) {
            printf("%2d ", j);
        }
        printf("\n");

        for (int i = 0; i < rows; i++) {
            printf("%2d ", i);
            for (int j = 0; j < cols; j++) {
                if (grid->highlighted[i][j] != ' ') {
                    printf("\033[1;32m%c\033[0m  ", grid->highlighted[i][j]);
                } else {
                    printf("%c  ", grid->letters[i][j]);
                }
            }
            printf("\n");
        }

        free(allPositions);
        free(counts);
        freeGrid(grid);
    } else {
        // Worker processes
        int rows, cols, numWords;
        MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

        Grid* grid = initializeGrid(rows, cols);
        for (int i = 0; i < rows; i++) {
            MPI_Bcast(grid->letters[i], cols, MPI_CHAR, 0, MPI_COMM_WORLD);
        }

        MPI_Bcast(&numWords, 1, MPI_INT, 0, MPI_COMM_WORLD);
        char words[MAX_WORDS][MAX_WORD_LENGTH];
        for (int i = 0; i < numWords; i++) {
            MPI_Bcast(words[i], MAX_WORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
        }

        int rowsPerProcess = rows / size;
        WordPosition* positions = (WordPosition*)malloc(MAX_WORDS * sizeof(WordPosition));
        int totalFound = 0;

        for (int w = 0; w < numWords; w++) {
            int count = 0;
            searchWordParallel(grid, words[w], rank * rowsPerProcess,
                             (rank + 1) * rowsPerProcess, positions + totalFound, &count);
            totalFound += count;
        }

        MPI_Gather(&totalFound, 1, MPI_INT, NULL, 1, MPI_INT, 0, MPI_COMM_WORLD);

        free(positions);
        freeGrid(grid);
    }

    MPI_Finalize();
    return 0;
}
