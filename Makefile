# Compiler and flags
MPICC = mpicc
CFLAGS = -Wall -Wextra -O3

# Program name
PROG = word_search

# Default number of processes
NP ?= 4

# Default input file
INPUT ?= puzzle.txt

# Timing test process counts
TIME_TESTS ?= 1 2 4 8

# Build targets
all: $(PROG)

# Compile the program
$(PROG): $(PROG).c
	$(MPICC) $(CFLAGS) -o $(PROG) $(PROG).c

# Run the program with default settings
run: $(PROG)
	mpirun -np $(NP) ./$(PROG) < $(INPUT)

# Run timing tests with different process counts
time-test: $(PROG)
	@echo "Running timing tests..."
	@for n in $(TIME_TESTS); do \
		echo "\nTest with $$n processes:"; \
		mpirun -np $$n ./$(PROG) < $(INPUT); \
	done

# Run with valgrind for memory checking
memcheck: $(PROG)
	mpirun -np $(NP) valgrind --leak-check=full ./$(PROG) < $(INPUT)

# Clean build files
clean:
	rm -f $(PROG)

# Help target
help:
	@echo "Available targets:"
	@echo "  make all       - Build the program (default)"
	@echo "  make run      - Run the program"
	@echo "  make time-test- Run timing tests with different process counts"
	@echo "  make memcheck - Run with valgrind memory checker"
	@echo "  make clean    - Remove built program"
	@echo ""
	@echo "Variables:"
	@echo "  NP=X          - Set number of processes (default: 4)"
	@echo "  INPUT=file    - Set input file (default: puzzle.txt)"
	@echo "  TIME_TESTS='1 2 4 8' - Set process counts for timing tests"
	@echo ""
	@echo "Example usage:"
	@echo "  make run NP=8 INPUT=custom_puzzle.txt"
	@echo "  make time-test TIME_TESTS='1 2 4 8 16'"

.PHONY: all run time-test memcheck clean help
