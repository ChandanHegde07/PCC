# PCC Compiler Makefile

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./include
LDFLAGS = -lm
DEBUG_FLAGS = -g -O0 -DPCC_DEBUG
RELEASE_FLAGS = -O2

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
TEST_DIR = tests
EXAMPLES_DIR = examples
OUTPUTS_DIR = outputs

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Test files
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS = $(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_DIR)/tests/%.o)

# Targets
TARGET = $(BUILD_DIR)/pcc
TEST_TARGET = $(BUILD_DIR)/test_runner

# Default target
all: $(TARGET)

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/tests

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -c $< -o $@

# Compile test files
$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -c $< -o $@

# Link main executable
$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Link test executable
$(TEST_TARGET): $(filter-out $(BUILD_DIR)/main.o,$(OBJS)) $(TEST_OBJS) | $(BUILD_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

# Run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Run compiler on example
run-example: $(TARGET)
	./$(TARGET) $(EXAMPLES_DIR)/valid_simple.pcc $(OUTPUTS_DIR)/output.json

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(OUTPUTS_DIR)/*.json
	rm -f $(OUTPUTS_DIR)/*.txt
	rm -f $(OUTPUTS_DIR)/*.md

# Install
install: $(TARGET)
	install -d $(PREFIX)/bin
	install -m 755 $(TARGET) $(PREFIX)/bin/pcc

# Uninstall
uninstall:
	rm -f $(PREFIX)/bin/pcc

# Help
help:
	@echo "PCC Compiler Makefile"
	@echo "====================="
	@echo "Targets:"
	@echo "  all         - Build the compiler (default)"
	@echo "  debug       - Build with debug symbols"
	@echo "  test        - Build and run tests"
	@echo "  run-example - Run compiler on example file"
	@echo "  clean       - Remove build artifacts"
	@echo "  install     - Install to PREFIX (default: /usr/local)"
	@echo "  uninstall   - Remove from PREFIX"
	@echo "  help        - Show this help message"

.PHONY: all debug test run-example clean install uninstall help
