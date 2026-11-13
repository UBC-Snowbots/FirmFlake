# Short wrapper around CMake presets. Real build logic lives in CMakeLists.txt.

BUILD_TYPE ?= Debug
BUILD_DIR  := build/$(BUILD_TYPE)
TARGET     := PDB_24V
ELF        := $(BUILD_DIR)/$(TARGET).elf
BIN        := $(BUILD_DIR)/$(TARGET).bin
SWD_FREQ   := 1M

.PHONY: all configure build bin flash-ocd clean rebuild flash reset help

all: build

help:
	@echo "make build                 Build Debug firmware"
	@echo "make BUILD_TYPE=Release build"
	@echo "make clean                 Clean the selected configuration"
	@echo "make rebuild               Clean and rebuild"
	@echo "make flash                 Build binary and flash with st-flash"
	@echo "make flash-ocd             Build and flash with OpenOCD"

configure:
	cmake --preset $(BUILD_TYPE)

build: configure
	cmake --build --preset $(BUILD_TYPE)

bin: build
	arm-none-eabi-objcopy -O binary $(ELF) $(BIN)

flash: bin
	st-flash --freq=$(SWD_FREQ) --reset write $(BIN) 0x08000000

flash-ocd: build
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg \
		-c "program $(ELF) verify reset exit"

reset:
	st-flash reset

clean:
	cmake --build --preset $(BUILD_TYPE) --target clean

rebuild: clean build