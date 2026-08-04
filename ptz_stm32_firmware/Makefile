# Thin wrapper around CMake. Real build logic lives in CMakeLists.txt.

BUILD_DIR   := build
TOOLCHAIN   := arm-none-eabi.cmake
BUILD_TYPE  ?= Debug 
TARGET      := ptz_stm32_firmware

.PHONY: all configure build flash-ocd clean rebuild flash reset help

all: build

help:
	@echo "\033[1;36m╔══════════════════════════════════════════════════╗\033[0m"
	@echo "\033[1;36m║     Peripheral Lighting Stack Firmware           ║\033[0m"
	@echo "\033[1;36m╚══════════════════════════════════════════════════╝\033[0m"
	@echo ""
	@echo "\033[1;33m  make build\033[0m        Configure and build firmware"
	@echo "\033[1;33m  make flash\033[0m        Build + flash via st-flash"
	@echo "\033[1;33m  make clean\033[0m        Nuke the build directory"
	@echo "\033[1;33m  cat Makefile\033[0m      See all available targets"
	@echo ""
	@echo "\033[0;90m  st-flash must be installed for flashing\033[0m"
	@echo ""

configure:
	cmake -B $(BUILD_DIR) -G Ninja \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure
	cmake --build $(BUILD_DIR)

flash: build
	st-flash --reset write $(BUILD_DIR)/$(TARGET).bin 0x08000000

flash-ocd: build
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg \
		-c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"
reset:
	st-flash reset

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build