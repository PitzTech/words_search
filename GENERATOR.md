# Word Search Puzzle Generator

The Word Search Puzzle Generator is a C-based tool that creates word search puzzles with customizable dimensions and word lists. The puzzles feature:

- Words placed in all directions (horizontal, vertical, diagonal).
- Words that wrap around the grid edges.
- Intersections between words for added complexity.

## File Structure

- `helpers/word_search.c`: The source code for the puzzle generator.
- `helpers/generated_puzzle.txt`: The output file where the generated puzzle is saved.

## Prerequisites

- A C compiler (e.g., `gcc`).
- A Makefile for building the project (included in the root directory).

## Usage Instructions

### Step 1: Navigate to the Helpers Directory
Navigate to the directory where the generator is located:
```bash
cd helpers
```

### Step 2: Compile the Program
Compile the program using the provided Makefile:
```bash
make
```

### Step 3: Run the Generator
Execute the program to generate a word search puzzle:
```bash
make run
```

### Step 4: View the Output
The generated word search puzzle will be saved in a file named `generated_puzzle.txt` in the same directory. Open the file to view the grid and the list of words.

#### Example Output in `generated_puzzle.txt`:
```
e u c e f e q v k j b d c n x e k z y o t g q n q s r m y d
z l w t n p f d z v u c r s z i s b x v r m m n x y h z z s
...

Palavras:
algoritmos, bubblesort, quicksort, mergesort, arvore, openmp, prova
```

### Step 5: Clean Up Files
To remove the compiled executable and the generated puzzle file, run:
```bash
make clean
```

## Customization

To customize the word search puzzle, edit the following variables in `word_search.c`:
- `height` and `width` for the grid dimensions.
- `words` array for the list of words to include in the puzzle.

## Logging

The program provides detailed logs during execution to indicate progress, such as initialization, word placement, and file writing.

#### Example Log Output:
```
Starting word search puzzle generation...
Allocating memory for grid...
Initializing grid...
Placing words in the grid...
Placing word: algoritmos
...
Puzzle written to 'generated_puzzle.txt'.
Finished word search puzzle generation.
```

## Features

- **Randomized Placement**: Words are placed at random starting points and in random directions.
- **Grid Wrapping**: Words can wrap around the edges of the grid.
- **Word Intersections**: Words may intersect when characters match.
- **Fill with Random Letters**: Empty grid cells are filled with random letters.

## Contributing

Feel free to submit issues or suggestions to enhance the Word Search Puzzle Generator.

---
