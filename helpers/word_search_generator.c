// word_search_generator.c
// This will help generate the large puzzle

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define GRID_SIZE 100
#define MAX_WORDS 25
#define MAX_WORD_LENGTH 50

typedef struct {
    char grid[GRID_SIZE][GRID_SIZE];
    char words[MAX_WORDS][MAX_WORD_LENGTH];
    int wordCount;
} Puzzle;

// Direction vectors for all 8 directions
const int dx[] = {-1, -1, -1, 0, 1, 1, 1, 0};
const int dy[] = {-1, 0, 1, 1, 1, 0, -1, -1};

void initGrid(Puzzle* puzzle) {
    // Initialize with random lowercase letters
    for(int i = 0; i < GRID_SIZE; i++) {
        for(int j = 0; j < GRID_SIZE; j++) {
            puzzle->grid[i][j] = 'a' + (rand() % 26);
        }
    }
}

int canPlaceWord(Puzzle* puzzle, const char* word, int row, int col, int dirIndex) {
    int len = strlen(word);
    int x = row, y = col;

    for(int i = 0; i < len; i++) {
        int newX = (x + GRID_SIZE) % GRID_SIZE;
        int newY = (y + GRID_SIZE) % GRID_SIZE;

        if(puzzle->grid[newX][newY] != 'a' + (rand() % 26) &&
           puzzle->grid[newX][newY] != word[i]) {
            return 0;
        }
        x += dx[dirIndex];
        y += dy[dirIndex];
    }
    return 1;
}

void placeWord(Puzzle* puzzle, const char* word, int row, int col, int dirIndex) {
    int len = strlen(word);
    int x = row, y = col;

    for(int i = 0; i < len; i++) {
        int newX = (x + GRID_SIZE) % GRID_SIZE;
        int newY = (y + GRID_SIZE) % GRID_SIZE;
        puzzle->grid[newX][newY] = word[i];
        x += dx[dirIndex];
        y += dy[dirIndex];
    }
}

void generatePuzzle(Puzzle* puzzle) {
    // List of words to place
    const char* wordList[] = {
        "parallelism", "multithreading", "concurrency", "optimization",
        "recursion", "synchronization", "hashtable", "linkedlist",
        "binarytree", "redblacktree", "arraylist", "binaryheap",
        "quicksort", "mergesort", "heapsort", "bubblesort",
        "insertionsort", "selectionsort", "radixsort", "virtualization",
        "compilation", "polymorphism", "encapsulation", "inheritance",
        "abstraction"
    };

    puzzle->wordCount = sizeof(wordList) / sizeof(wordList[0]);

    // Copy words to puzzle
    for(int i = 0; i < puzzle->wordCount; i++) {
        strcpy(puzzle->words[i], wordList[i]);
    }

    // Initialize grid with random letters
    initGrid(puzzle);

    // Place each word
    for(int i = 0; i < puzzle->wordCount; i++) {
        int placed = 0;
        int attempts = 0;

        while(!placed && attempts < 100) {
            int row = rand() % GRID_SIZE;
            int col = rand() % GRID_SIZE;
            int dir = rand() % 8;

            if(canPlaceWord(puzzle, wordList[i], row, col, dir)) {
                placeWord(puzzle, wordList[i], row, col, dir);
                placed = 1;
            }
            attempts++;
        }
    }
}

void printPuzzle(const Puzzle* puzzle, const char* filename) {
    FILE* file = fopen(filename, "w");
    if(!file) {
        printf("Error opening file\n");
        return;
    }

    // Print grid
    for(int i = 0; i < GRID_SIZE; i++) {
        for(int j = 0; j < GRID_SIZE; j++) {
            fprintf(file, "%c ", puzzle->grid[i][j]);
        }
        fprintf(file, "\n");
    }

    // Print word list
    fprintf(file, "\nPalavras:\n");
    for(int i = 0; i < puzzle->wordCount; i++) {
        fprintf(file, "%s", puzzle->words[i]);
        if(i < puzzle->wordCount - 1) {
            fprintf(file, ", ");
        }
    }
    fprintf(file, "\n");

    fclose(file);
}

int main() {
    srand(time(NULL));
    Puzzle puzzle;
    generatePuzzle(&puzzle);
    printPuzzle(&puzzle, "large_puzzle.txt");
    return 0;
}
