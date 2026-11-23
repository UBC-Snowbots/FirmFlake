BUILD_DIR   := build
TOOLCHAIN   := cmake/gcc-arm-none-eabi.cmake
BUILD_TYPE  ?= Debug 

.PHONY: all configure build clean test rebuild

all: build

configure:
	cmake -B $(BUILD_DIR) -G Ninja \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure
	cmake --build $(BUILD_DIR)

clean:
	cmake -E rm -rf $(BUILD_DIR)
rebuild: clean all