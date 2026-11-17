# Makefile common libraries (inspired by AI, need to test on linux, working on windows)
# Invokes cmake commands, rather than platform specific commands suchas rm -rf on linux
# !forces Ninja

BUILD_DIR := build

.PHONY: all configure build clean test rebuild

all: build test

configure:
	cmake -S . -B $(BUILD_DIR) -G Ninja 

build: configure
	cmake --build $(BUILD_DIR)

clean:
	cmake -E rm -rf $(BUILD_DIR)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

rebuild: clean all


# # Makefile for common libraries (old for reference)

# BUILD_DIR:=build

# all: clean configure build test

# configure:
# 	@mkdir -p $(BUILD_DIR)
# 	@cd $(BUILD_DIR) && cmake ..

# build: configure 
# 	@$(MAKE) -C $(BUILD_DIR)

# clean:
# 	@rm -rf $(BUILD_DIR)

# test: configure build
# 	@${BUILD_DIR}/common_lib_tests


# .PHONY: all configure build clean test