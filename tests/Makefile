CXX ?= c++
CXXFLAGS ?= -O2 -Wall -Wextra -Wconversion -Werror
SOURCES = firmware_checks.cpp ../src/controls.cpp ../src/delay.cpp
HEADERS = ../src/controls.h ../src/delay.h
BUILD_DIR = ../build/host-tests
TEST_BIN = $(BUILD_DIR)/firmware_checks

.PHONY: check
check: $(TEST_BIN)
	$(TEST_BIN)

$(TEST_BIN): $(SOURCES) $(HEADERS) Makefile
	mkdir -p $(BUILD_DIR)
	$(CXX) -std=c++17 $(CPPFLAGS) $(CXXFLAGS) -I../src $(SOURCES) $(LDFLAGS) -o $@
