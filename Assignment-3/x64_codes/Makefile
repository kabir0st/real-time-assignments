# Makefile for x86_64 Linux version of TinyThreads
# Adapted from Raspberry Pi version for testing on WSL2/Linux

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -g -Ilib
LDFLAGS = -lm

# Target executable name
TARGET = main

# Source files
MAIN_SRC = main.c
LIB_SRC = lib/tinythreads.c lib/expstruct.c

# Object files
MAIN_OBJ = main.o
LIB_OBJ = lib/tinythreads.o lib/expstruct.o

# All object files
OBJS = $(MAIN_OBJ) $(LIB_OBJ)

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete! Run with: ./$(TARGET)"

# Compile main.c
$(MAIN_OBJ): $(MAIN_SRC) lib/tinythreads.h lib/expstruct.h
	@echo "Compiling $(MAIN_SRC)..."
	$(CC) $(CFLAGS) -c $(MAIN_SRC) -o $(MAIN_OBJ)

# Compile lib/tinythreads.c
lib/tinythreads.o: lib/tinythreads.c lib/tinythreads.h
	@echo "Compiling lib/tinythreads.c..."
	$(CC) $(CFLAGS) -c lib/tinythreads.c -o lib/tinythreads.o

# Compile lib/expstruct.c
lib/expstruct.o: lib/expstruct.c lib/expstruct.h
	@echo "Compiling lib/expstruct.c..."
	$(CC) $(CFLAGS) -c lib/expstruct.c -o lib/expstruct.o

# Run target - builds and runs the program
run: $(TARGET)
	@echo ""
	@echo "=========================================="
	@echo "Running TinyThreads x64 Linux Test"
	@echo "=========================================="
	@echo ""
	./$(TARGET)

# Test target - same as run
test: run

# Clean up object files and executable
clean:
	@echo "Cleaning up..."
	rm -f $(OBJS) $(TARGET)
	@echo "Clean complete!"

# Rebuild everything from scratch
rebuild: clean all

# Debug info
info:
	@echo "Compiler: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "Source files: $(MAIN_SRC) $(LIB_SRC)"
	@echo "Object files: $(OBJS)"
	@echo "Target: $(TARGET)"

# Phony targets
.PHONY: all run test clean rebuild info

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build the executable (default)"
	@echo "  run     - Build and run the program"
	@echo "  test    - Same as run"
	@echo "  clean   - Remove object files and executable"
	@echo "  rebuild - Clean and rebuild everything"
	@echo "  info    - Show build configuration"
	@echo "  help    - Show this help message"
