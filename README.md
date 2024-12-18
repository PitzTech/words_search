# Parallel Word Search Solver

A high-performance C program that solves word search puzzles using parallel processing with MPI (Message Passing Interface). The program can find words in any direction, including diagonals and wrapping around the edges of the puzzle.

## Documentation
- [How it Works](./HOW_IT_WORKS.md) - Detailed explanation of the algorithm and parallelization strategy
- [Generator](./GENERATOR.md) - Documentation for the puzzle generator tool

## Features
- Parallel processing using MPI
- Support for all 8 directions (horizontal, vertical, diagonal)
- Words can wrap around grid edges
- Automatic grid size detection
- Case-insensitive word matching
- Color-coded output highlighting
- Performance metrics

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

# Run performance timing tests
make time-test

# Run custom timing tests
make time-test TIME_TESTS='1 2 4 8 16'

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
- `TIME_TESTS`: Process counts for timing tests (default: 1 2 4 8)

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

4. Performance metrics:
```
Execution time: X.XXXX seconds
Using N processes
```

## Performance Testing

The program includes built-in performance timing:

1. Basic timing:
```bash
make run
# Shows execution time for single run
```

2. Comparative timing tests:
```bash
make time-test
# Runs tests with 1, 2, 4, and 8 processes
```

3. Custom timing tests:
```bash
make time-test TIME_TESTS='1 2 4 8 16 32'
# Tests with specified number of processes
```

Performance considerations:
- Optimal process count typically matches CPU core count
- Grid size affects optimal process distribution
- Large grids benefit more from parallelization
- Small grids may see overhead with too many processes

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
4. Consider grid size when choosing process count
5. Monitor execution times to find optimal configuration

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

4. If performance is poor:
   ```bash
   # Run timing tests to find optimal process count
   make time-test
   ```

## Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is open source and available under the MIT License.


