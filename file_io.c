#include "file_io.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
