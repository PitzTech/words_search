CC = mpicc
CFLAGS = -Wall -Wextra -O3
LDFLAGS = -lm

# Directories
BUILD_DIR = build
SRC_DIR = .

# Source files
SRCS = main.c grid.c search.c file_io.c mpi_handler.c output.c debug.c constants.c
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)
HEADERS = $(wildcard *.h)

# Program name
PROG = $(BUILD_DIR)/word_search

# Default number of processes
NP ?= 4

# Default input file
INPUT ?= puzzle.txt

# Timing test process counts
TIME_TESTS ?= 1 2 4 8

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link the program
$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(PROG)

run: $(PROG)
	mpirun -np $(NP) ./$(PROG) < $(INPUT)

time-test: $(PROG)
	@echo "Running timing tests..."
	@for n in $(TIME_TESTS); do \
		echo "\nTest with $$n processes:"; \
		mpirun -np $$n ./$(PROG) < $(INPUT); \
	done

memcheck: $(PROG)
	mpirun -np $(NP) valgrind --leak-check=full ./$(PROG) < $(INPUT)

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Available targets:"
	@echo "  make all       - Build the program (default)"
	@echo "  make run      - Run the program"
	@echo "  make time-test- Run timing tests with different process counts"
	@echo "  make memcheck - Run with valgrind memory checker"
	@echo "  make clean    - Remove build directory"
	@echo ""
	@echo "Variables:"
	@echo "  NP=X          - Set number of processes (default: 4)"
	@echo "  INPUT=file    - Set input file (default: puzzle.txt)"
	@echo "  TIME_TESTS='1 2 4 8' - Set process counts for timing tests"
	@echo ""
	@echo "Example usage:"
	@echo "  make run NP=8 INPUT=custom_puzzle.txt"
	@echo "  make time-test TIME_TESTS='1 2 4 8 16'"

.PHONY: all run time-test memcheck clean help $(BUILD_DIR)
