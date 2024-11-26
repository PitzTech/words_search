# Compiler and flags
MPICC = mpicc
CFLAGS = -Wall -Wextra -O3

# Program name
PROG = word_search

# Default number of processes
NP ?= 4

# Default input file
INPUT ?= puzzle.txt

# Build targets
all: $(PROG)

# Compile the program
$(PROG): $(PROG).c
	$(MPICC) $(CFLAGS) -o $(PROG) $(PROG).c

# Run the program with default settings
run: $(PROG)
	mpirun -np $(NP) ./$(PROG) < $(INPUT)

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
	@echo "  make memcheck - Run with valgrind memory checker"
	@echo "  make clean    - Remove built program"
	@echo ""
	@echo "Variables:"
	@echo "  NP=X          - Set number of processes (default: 4)"
	@echo "  INPUT=file    - Set input file (default: puzzle.txt)"
	@echo ""
	@echo "Example usage:"
	@echo "  make run NP=8 INPUT=custom_puzzle.txt"

.PHONY: all run memcheck clean help
