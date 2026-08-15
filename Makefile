# Relay-to-RS485 build entry point.
#
# PlatformIO-free: wraps arduino-cli for firmware builds and the host
# toolchain for unit tests.
#
#   make            # build firmware
#   make upload     # build and flash over USB
#   make test       # host unit tests (no Arduino needed)
#   make clean      # remove build artifacts
#
# Prerequisites: arduino-cli on PATH, ESP32 core installed:
#   arduino-cli core install esp32:esp32
#   arduino-cli lib install ModbusMaster@2.0.1

FQBN        := esp32:esp32:lolin_s2_mini
SKETCH      := relay_to_rs485
TEST_SRC    := test/test_debounce.cpp
TEST_BIN    := build/test_debounce
CXX         ?= g++
CXXFLAGS    := -std=c++11 -Wall -Wextra -Werror -I relay_to_rs485

.PHONY: all build upload test clean

all: build

build:
	arduino-cli compile --fqbn $(FQBN) $(SKETCH)

upload:
	arduino-cli compile --fqbn $(FQBN) $(SKETCH) --upload

$(TEST_BIN): $(TEST_SRC) relay_to_rs485/debounce.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(TEST_SRC) -o $(TEST_BIN)

test: $(TEST_BIN)
	$(TEST_BIN)

clean:
	rm -rf build
