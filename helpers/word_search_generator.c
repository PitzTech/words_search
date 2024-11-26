#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DIRECTIONS_COUNT 8

typedef struct {
    int dx;
    int dy;
    char *name;
} DirectionVector;

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

// Function prototypes
void initializeGrid(char **grid, int height, int width);
void fillGridWithRandomChars(char **grid, int height, int width);
void printGridToFile(FILE *file, char **grid, int height, int width);
int placeWord(char **grid, int height, int width, const char *word);
void wrapAround(int *x, int *y, int height, int width);
int canPlaceWord(char **grid, int height, int width, const char *word, int startX, int startY, int dirIndex);
void placeWordInDirection(char **grid, int height, int width, const char *word, int startX, int startY, int dirIndex);

int main() {
    int height = 100;
    int width = 100;
    char *words[] = {"algoritmos", "bubblesort", "quicksort", "mergesort", "arvore", "openmp", "prova", "teste"};
    int wordCount = sizeof(words) / sizeof(words[0]);

    printf("Starting word search puzzle generation...\n");

    // Allocate grid memory
    printf("Allocating memory for grid...\n");
    char **grid = (char **)malloc(height * sizeof(char *));
    for (int i = 0; i < height; i++) {
        grid[i] = (char *)malloc(width * sizeof(char));
    }

    // Initialize grid with '.'
    printf("Initializing grid...\n");
    initializeGrid(grid, height, width);

    // Seed random generator
    printf("Seeding random number generator...\n");
    srand(time(NULL));

    // Place words in the grid
    printf("Placing words in the grid...\n");
    for (int i = 0; i < wordCount; i++) {
        printf("Placing word: %s\n", words[i]);
        if (!placeWord(grid, height, width, words[i])) {
            printf("Failed to place word: %s\n", words[i]);
        }
    }

    // Fill the grid with random letters
    printf("Filling empty cells with random characters...\n");
    fillGridWithRandomChars(grid, height, width);

    // Write grid and words to a file
    printf("Writing puzzle to file 'generated_puzzle.txt'...\n");
    FILE *file = fopen("generated_puzzle.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    printGridToFile(file, grid, height, width);

    fprintf(file, "\nPalavras:\n");
    for (int i = 0; i < wordCount; i++) {
        fprintf(file, "%s%s", words[i], i < wordCount - 1 ? ", " : "\n");
    }

    fclose(file);
    printf("Puzzle written to 'generated_puzzle.txt'.\n");

    // Free grid memory
    printf("Freeing allocated memory...\n");
    for (int i = 0; i < height; i++) {
        free(grid[i]);
    }
    free(grid);

    printf("Finished word search puzzle generation.\n");
    return 0;
}

void initializeGrid(char **grid, int height, int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            grid[i][j] = '.';
        }
    }
}

void fillGridWithRandomChars(char **grid, int height, int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (grid[i][j] == '.') {
                grid[i][j] = 'a' + rand() % 26;
            }
        }
    }
}

void printGridToFile(FILE *file, char **grid, int height, int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fprintf(file, "%c ", grid[i][j]);
        }
        fprintf(file, "\n");
    }
}

void wrapAround(int *x, int *y, int height, int width) {
    *x = (*x + height) % height;
    *y = (*y + width) % width;
}

int canPlaceWord(char **grid, int height, int width, const char *word, int startX, int startY, int dirIndex) {
    int x = startX, y = startY;
    for (int i = 0; word[i] != '\0'; i++) {
        wrapAround(&x, &y, height, width);
        if (grid[x][y] != '.' && grid[x][y] != word[i]) {
            return 0; // Conflict
        }
        x += DIRECTION_VECTORS[dirIndex].dx;
        y += DIRECTION_VECTORS[dirIndex].dy;
    }
    return 1;
}

void placeWordInDirection(char **grid, int height, int width, const char *word, int startX, int startY, int dirIndex) {
    int x = startX, y = startY;
    for (int i = 0; word[i] != '\0'; i++) {
        wrapAround(&x, &y, height, width);
        grid[x][y] = word[i];
        x += DIRECTION_VECTORS[dirIndex].dx;
        y += DIRECTION_VECTORS[dirIndex].dy;
    }
}

int placeWord(char **grid, int height, int width, const char *word) {
    int attempts = 1000; // Limit attempts to avoid infinite loops
    while (attempts--) {
        int startX = rand() % height;
        int startY = rand() % width;
        int dirIndex = rand() % DIRECTIONS_COUNT;

        if (canPlaceWord(grid, height, width, word, startX, startY, dirIndex)) {
            placeWordInDirection(grid, height, width, word, startX, startY, dirIndex);
            return 1; // Success
        }
    }
    return 0; // Failed to place the word
}
