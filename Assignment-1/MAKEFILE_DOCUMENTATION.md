# Makefile Documentation

## Overview

Both projects (a1p1 and a1p2) use similar Makefiles configured for bare-metal ARM cross-compilation targeting Raspberry Pi 3. The Makefiles follow GNU Make conventions and are designed for embedded systems development.

## Common Configuration

### Toolchain Setup
```makefile
CROSS   = arm-none-eabi-
CC      = $(CROSS)gcc
AS      = $(CROSS)as
SIZE    = $(CROSS)size
OCOPY   = $(CROSS)objcopy
```

**Toolchain Components**:
- **arm-none-eabi-gcc**: Cross-compiler for ARM Cortex-A processors
- **arm-none-eabi-as**: Cross-assembler for ARM assembly
- **arm-none-eabi-size**: Displays section sizes of object files
- **arm-none-eabi-objcopy**: Converts between object file formats

### Compiler Flags

#### CFLAGS (C Compiler Flags)
```makefile
CFLAGS = -march=armv8-a+crc -mtune=cortex-a53 -mfpu=vfp -mfloat-abi=soft \
         -ffunction-sections -fdata-sections -fno-common -g -std=gnu99 \
         -Wall -Wextra -Os -Ilib -DRPI3=1 -DIOBPLUS=1
```

**Flag Breakdown**:
- **`-march=armv8-a+crc`**: Target ARMv8-A architecture with CRC extensions
- **`-mtune=cortex-a53`**: Optimize for Cortex-A53 processor (RPi3 CPU)
- **`-mfpu=vfp`**: Use Vector Floating Point unit
- **`-mfloat-abi=soft`**: Use software floating-point ABI
- **`-ffunction-sections`**: Place each function in separate section
- **`-fdata-sections`**: Place each data item in separate section
- **`-fno-common`**: Do not place uninitialized globals in common section
- **`-g`**: Include debugging information
- **`-std=gnu99`**: Use GNU C99 standard
- **`-Wall -Wextra`**: Enable comprehensive warnings
- **`-Os`**: Optimize for size
- **`-Ilib`**: Include library directory in header search path
- **`-DRPI3=1 -DIOBPLUS=1`**: Define preprocessor macros for platform

#### LFLAGS (Linker Flags)
```makefile
LFLAGS = -static -nostartfiles -lc -lgcc -specs=nano.specs -Wl,--gc-sections -lm
```

**Flag Breakdown**:
- **`-static`**: Create statically linked executable
- **`-nostartfiles`**: Do not use standard system startup files
- **`-lc`**: Link with C standard library
- **`-lgcc`**: Link with GCC runtime library
- **`-specs=nano.specs`**: Use newlib-nano (smaller C library)
- **`-Wl,--gc-sections`**: Remove unused sections during linking
- **`-lm`**: Link with math library

#### LDFLAGS (Additional Linker Flags)
```makefile
LDFLAGS += -u _printf_float
```
- **`-u _printf_float`**: Force inclusion of floating-point printf support

### Linker Script
```makefile
LSCRIPT = lib/rpi3.ld
```
- Defines memory layout and section placement for Raspberry Pi 3
- Critical for bare-metal applications to ensure proper memory mapping

## Project-Specific Configurations

### A1P1 Configuration

#### Main File and Objects
```makefile
MAINFILE = a1p1

OBJS  = lib/iregister.o
OBJS += lib/uart.o lib/rpi-armtimer.o lib/rpi-gpio.o lib/rpi-interrupts.o lib/rpi-systimer.o
OBJS += lib/startup.o lib/syscalls.o
OBJS += $(MAINFILE).o
```

**Object Files**:
- **`iregister.o`**: Integer register manipulation functions
- **`uart.o`**: UART communication library
- **`rpi-armtimer.o`**: ARM timer hardware abstraction
- **`rpi-gpio.o`**: GPIO hardware abstraction
- **`rpi-interrupts.o`**: Interrupt handling
- **`rpi-systimer.o`**: System timer functions
- **`startup.o`**: Boot and initialization code
- **`syscalls.o`**: System call implementations
- **`a1p1.o`**: Main application object

### A1P2 Configuration

#### Main File and Objects
```makefile
MAINFILE = a1p2

OBJS  = lib/led.o lib/rpi-gpio.o lib/rpi-armtimer.o lib/rpi-interrupts.o lib/rpi-systimer.o
OBJS += lib/startup.o lib/syscalls.o
OBJS += $(MAINFILE).o
```

**Key Differences from A1P1**:
- **`led.o`**: LED control functions (instead of iregister.o and uart.o)
- Smaller footprint - no UART communication required
- Same core hardware abstraction libraries

## Build Targets

### Default Target
```makefile
all: $(MAIN)
```
- Builds the complete binary image file ready for deployment

### File Generation Flow

#### 1. Object File Compilation
```makefile
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^
```
- **Pattern Rule**: Compiles any `.c` file to corresponding `.o` file
- **`-c`**: Compile only, do not link
- **`$@`**: Target file name (`*.o`)
- **`$^`**: All prerequisites (`*.c`)

#### 2. ELF File Generation
```makefile
$(ELF): $(OBJS)
	$(CC) -T $(LSCRIPT) $(CFLAGS) $(LFLAGS) $(LDFLAGS) -o $@ $^
	$(SIZE) $@
```
- **Links**: All object files into executable ELF format
- **`-T $(LSCRIPT)`**: Use custom linker script
- **`$(SIZE) $@`**: Display section sizes after linking

#### 3. Binary Image Creation
```makefile
$(MAIN): $(ELF)
	$(OCOPY) $< -O binary $@
```
- **Converts**: ELF file to raw binary format
- **`-O binary`**: Output format specification
- **Result**: `.img` file ready for Raspberry Pi boot

### Output Files

#### Generated Files per Project
- **`a1p1.o` / `a1p2.o`**: Main application object file
- **`a1p1.elf` / `a1p2.elf`**: Linked executable with debug symbols
- **`a1p1.img` / `a1p2.img`**: Raw binary image for deployment

### Clean Target
```makefile
clean:
#   OS dependent. Change accordingly
#   del /Q /F $(MAIN) $(ELF) $(OBJS)
#   rm -f $(MAIN) $(ELF) $(OBJS)
```
- **Commented Out**: Allows for OS-specific cleanup commands
- **Windows**: `del /Q /F` command
- **Unix/Linux**: `rm -f` command

### Run Target
```makefile
run: $(MAIN)
```
- **Empty Target**: Depends on binary being built
- **Extension Point**: Could be customized for deployment automation

## Special Makefile Features

### Phony Targets
```makefile
.PHONY: all clean run
```
- **Purpose**: Declares targets that don't create files
- **Benefit**: Prevents conflicts with files named 'all', 'clean', or 'run'

### Variable Expansion
- **`$(MAINFILE)`**: Expands to project-specific main file name
- **`$(OBJS)`**: Expands to complete list of object files
- **`$(CFLAGS)`**: Expands to all compiler flags

### Cross-Platform Considerations
- **Toolchain Prefix**: `arm-none-eabi-` ensures ARM cross-compilation
- **Soft Float ABI**: Compatible with systems without hardware FPU
- **Nano Specs**: Reduces binary size for embedded applications

## Build Process Flow

1. **Source Analysis**: Make determines dependencies from source files
2. **Object Compilation**: Each `.c` file compiled to `.o` with full flags
3. **Library Integration**: All library objects included in link step
4. **ELF Linking**: Complete executable created with custom memory layout
5. **Size Report**: Section sizes displayed for memory usage analysis
6. **Binary Conversion**: Raw binary extracted for direct hardware execution

## Memory Layout Considerations

### Linker Script Importance
- **Memory Mapping**: Defines where code/data is placed in RPi3 memory
- **Boot Requirements**: Ensures proper entry point for bare-metal boot
- **Section Organization**: Separates code, data, and BSS sections

### Size Optimization
- **Function Sections**: Enables dead code elimination
- **Data Sections**: Allows unused data removal
- **Garbage Collection**: `--gc-sections` removes unreferenced code
- **Nano Library**: Smaller C library implementation

## Troubleshooting Build Issues

### Common Problems
1. **Missing Toolchain**: Ensure `arm-none-eabi-*` tools are installed
2. **Include Paths**: Verify `-Ilib` flag includes necessary headers
3. **Linker Script**: Ensure `lib/rpi3.ld` exists and is valid
4. **Object Dependencies**: Check that all required `.c` files are present

### Debug Information
- **ELF Size Output**: Shows memory usage breakdown
- **Debug Symbols**: `-g` flag enables GDB debugging
- **Warning Messages**: `-Wall -Wextra` helps catch potential issues

This Makefile system provides a robust foundation for bare-metal Raspberry Pi development with optimized builds and clear dependency management.
