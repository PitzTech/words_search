CC = gcc
CFLAGS = -Wall -O2
TARGET = word_search_generator

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) generated_puzzle.txt
