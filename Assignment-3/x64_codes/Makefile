# Simple Makefile for compiling and testing main.c

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS =

# Target executable name
TARGET = main

# Source files
MAIN_SRC = main.c
LIB_SRC = lib/tinythreads.c lib/expstruct.c

# Object files
MAIN_OBJ = main.o
LIB_OBJ = lib/tinythreads.o lib/expstruct.o

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(MAIN_OBJ) $(LIB_OBJ)
	$(CC) $(MAIN_OBJ) $(LIB_OBJ) -o $(TARGET) $(LDFLAGS)

# Compile main.c
$(MAIN_OBJ): $(MAIN_SRC) lib/tinythreads.h lib/expstruct.h
	$(CC) $(CFLAGS) -c $(MAIN_SRC) -o $(MAIN_OBJ)

# Compile lib/tinythreads.c
lib/tinythreads.o: lib/tinythreads.c lib/tinythreads.h
	$(CC) $(CFLAGS) -c lib/tinythreads.c -o lib/tinythreads.o

# Compile lib/expstruct.c
lib/expstruct.o: lib/expstruct.c lib/expstruct.h
	$(CC) $(CFLAGS) -c lib/expstruct.c -o lib/expstruct.o

# Test target - runs the program
test: $(TARGET)
	@echo "Running $(TARGET)..."
	./$(TARGET)

# Clean up object files and executable
clean:
	rm -f $(MAIN_OBJ) $(LIB_OBJ) $(TARGET)

# Phony targets
.PHONY: all test clean

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build the executable (default)"
	@echo "  test    - Build and run the program"
	@echo "  clean   - Remove object files and executable"
	@echo "  help    - Show this help message"
