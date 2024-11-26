CC = mpicc
CFLAGS = -Wall -Wextra -O3
LDFLAGS = -lm

# Directories
BUILD_DIR = build
SRC_DIR = .
EXPORT_DIR = exports

# Source files
SRCS = main.c grid.c search.c file_io.c mpi_handler.c output.c debug.c constants.c
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)
HEADERS = $(wildcard *.h)

# Program name
PROG = $(BUILD_DIR)/word_search

# Default parameters
NP ?= 4
INPUT ?= puzzle.txt
TIME_TESTS ?= 1 2 4 8
OUTPUT ?=
HTML ?=

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

# Add creation of export directory
$(EXPORT_DIR):
	mkdir -p $(EXPORT_DIR)

# Main run target with output options
run: $(PROG) $(EXPORT_DIR)
	mpirun -np $(NP) ./$(PROG) $(if $(OUTPUT),-o $(EXPORT_DIR)/$(OUTPUT)) $(if $(HTML),--html) < $(INPUT)

# Run timing tests
time-test: $(PROG)
	@echo "Running timing tests..."
	@for n in $(TIME_TESTS); do \
		echo "\nTest with $$n processes:"; \
		mpirun -np $$n ./$(PROG) $(if $(OUTPUT),-o output_$$n.txt) $(if $(HTML),--html) < $(INPUT); \
	done

# Memory check
memcheck: $(PROG)
	mpirun -np $(NP) valgrind --leak-check=full ./$(PROG) $(if $(OUTPUT),-o $(OUTPUT)) $(if $(HTML),--html) < $(INPUT)

clean:
	rm -rf $(BUILD_DIR) $(EXPORT_DIR)

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
	@echo "  OUTPUT=file   - Set output file (optional)"
	@echo "  HTML=yes      - Use HTML format for output (optional)"
	@echo "  TIME_TESTS='1 2 4 8' - Set process counts for timing tests"
	@echo ""
	@echo "Example usage:"
	@echo "  make run NP=8 INPUT=custom_puzzle.txt"
	@echo "  make run NP=4 INPUT=puzzle.txt OUTPUT=results.html HTML=yes"
	@echo "  make time-test TIME_TESTS='1 2 4 8 16'"

.PHONY: all run time-test memcheck clean help $(BUILD_DIR)
