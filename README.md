# Parallel Word Search Puzzle Solver

A high-performance C program that solves word search puzzles using parallel processing with MPI (Message Passing Interface). The program can find words in any direction, including diagonals and wrapping around the edges of the puzzle.

## Features

- Parallel processing using MPI for improved performance
- Supports all 8 directions for word search:
  ```
  Up            (-1,  0)
  Down          ( 1,  0)
  Left          ( 0, -1)
  Right         ( 0,  1)
  Up-Left       (-1, -1)
  Up-Right      (-1,  1)
  Down-Left     ( 1, -1)
  Down-Right    ( 1,  1)
  ```
- Words can wrap around edges
- Automatic grid size detection
- Case-insensitive word matching
- Color-coded output highlighting found words
- Reports exact positions of found words

## Project Structure

```
word_search/
│
├── Makefile            # Build configuration
├── word_search.c       # Source code
├── puzzle.txt          # Example puzzle
└── README.md          # Documentation
```

## Prerequisites

### Ubuntu (WSL 2)
```bash
# Update package lists
sudo apt update

# Install build essentials (gcc, make, etc.)
sudo apt install build-essential

# Install OpenMPI
sudo apt install openmpi-bin openmpi-common libopenmpi-dev
```

### Windows
1. Download and install Microsoft MPI:
   - [MS-MPI v10.1.1](https://www.microsoft.com/en-us/download/details.aspx?id=100593)
   - Install both MSMpiSetup.exe and msmpisdk.msi
2. Add MPI to your PATH:
   - Add `C:\Program Files\Microsoft MPI\Bin` to system PATH

## Input Format (puzzle.txt)

```
[Grid of letters separated by spaces]
[Empty line]
Palavras:
[Comma or space-separated list of words to find]
```

Example:
```
e u c e f e q v k j
z l w t n p f d z v
x s a m g d u d i b
...

Palavras:
algoritmos, bubblesort, quicksort, mergesort
```

## Compilation and Execution

The project includes a Makefile with several targets:

### Basic Commands

```bash
# Compile the program
make

# Run with default settings (4 processes)
make run

# Run with specific number of processes
make run NP=8

# Run with different input file
make run INPUT=custom_puzzle.txt

# Run with both custom processes and input
make run NP=6 INPUT=custom_puzzle.txt

# Check for memory leaks
make memcheck

# Clean build files
make clean

# Show help
make help
```

### Makefile Options

- `NP`: Number of processes (default: 4)
- `INPUT`: Input file path (default: puzzle.txt)
- `CFLAGS`: Compiler flags (-Wall -Wextra -O3)

## Output Format

The program outputs:
1. Grid dimensions and word list:
```
Your grid dimensions are X columns x Y rows
The number of words to search is: N
The words are: word1, word2, word3, ...
```

2. Results grid with:
   - Row and column numbers
   - Found words highlighted in green
   - Original letters in white

3. Found words list with positions:
```
Found words:
word1: (row1,col1) to (row2,col2)
word2: (row3,col3) to (row4,col4)
...
```

## Error Handling

The program handles various errors:
- Memory allocation failures
- File reading errors
- Invalid input format
- MPI communication errors

## Performance Optimization

For best performance:
1. Use number of processes (NP) matching your CPU cores
2. Compile with optimization (-O3 flag, included in Makefile)
3. Ensure input file is properly formatted

## Limitations

- MAX_WORD_LENGTH: 50 characters
- MAX_WORDS: 100 words
- MAX_LINE_LENGTH: 2000 characters

## Development

Key source code sections:
```c
// Direction enumeration
typedef enum {
    DIR_UP,
    DIR_DOWN,
    // ...
} Direction;

// Grid structure
typedef struct {
    char** letters;
    int rows;
    int cols;
    char** highlighted;
} Grid;

// Word position structure
typedef struct {
    int startRow;
    int startCol;
    int endRow;
    int endCol;
    char word[MAX_WORD_LENGTH];
} WordPosition;
```

## Troubleshooting

1. If compilation fails:
   ```bash
   # Check MPI installation
   mpicc --version
   ```

2. If execution fails:
   ```bash
   # Verify input file format
   cat puzzle.txt

   # Check process count
   echo $NP
   ```

3. If memory leaks occur:
   ```bash
   make memcheck
   ```

## Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is open source and available under the MIT License.
