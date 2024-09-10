# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I$(INC_DIR)
LDFLAGS =

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
LIB_DIR = lib
TEST_DIR = test

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/test_%.o,$(TEST_SRCS))

# Executable names
TARGET = mydb
TEST_TARGET = run_tests

# Default target
all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)

# Rule to create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Rule to create the main executable
$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Rule to compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Test target
test: $(BUILD_DIR) $(BUILD_DIR)/$(TEST_TARGET)

# Rule to create test executable
$(BUILD_DIR)/$(TEST_TARGET): $(filter-out $(BUILD_DIR)/main.o, $(OBJS)) $(TEST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Rule to compile test files
$(BUILD_DIR)/test_%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

# Clean rule
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean test
