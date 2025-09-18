# A1P1 - Integer Register Manipulation

## Overview

This project implements a comprehensive integer register manipulation system that runs on bare-metal Raspberry Pi 3. It demonstrates bit-level operations on a 32-bit register through an interactive UART interface.

## Features

The application provides an interactive menu system for testing various bit manipulation operations:

1. **Bit Operations**:
   - Reset individual bits
   - Set individual bits
   - Read individual bits
   - Reset all bits
   - Set all bits

2. **Nibble Operations**:
   - Get nibble values (4-bit groups)
   - Assign nibble values

3. **Shift Operations**:
   - Left shift with zero fill
   - Right shift (logical) with zero fill

4. **Display Features**:
   - Binary representation of register content
   - Decimal value display
   - Interactive UART communication

## File Structure

### Main Application
- **`a1p1.c`** - Main application with UART-based user interface

### Core Libraries
- **`lib/iregister.h`** - Header file defining the iRegister structure and function prototypes
- **`lib/iregister.c`** - Implementation of all bit manipulation functions
- **`lib/uart.h`** - UART communication interface definitions
- **`lib/uart.c`** - UART communication implementation

### Hardware Abstraction
- **`lib/rpi-gpio.h`** - GPIO peripheral definitions and control functions
- **`lib/rpi-base.h`** - Base hardware definitions and register types

### Build System
- **`Makefile`** - Build configuration for ARM cross-compilation

## Core Data Structure

### iRegister
```c
typedef struct {
    int content;  // 32-bit integer representing the register
} iRegister;
```

The iRegister structure encapsulates a 32-bit integer that can be manipulated at the bit level.

## Key Functions

### Bit Manipulation Functions

#### `resetBit(int i, iRegister *r)`
- **Purpose**: Sets the i-th bit to 0
- **Parameters**:
  - `i`: Bit position (0-31)
  - `r`: Pointer to iRegister
- **Implementation**: Uses bitwise AND with complement: `r->content &= ~(1 << i)`

#### `setBit(int i, iRegister *r)`
- **Purpose**: Sets the i-th bit to 1
- **Parameters**:
  - `i`: Bit position (0-31)
  - `r`: Pointer to iRegister
- **Implementation**: Uses bitwise OR: `r->content |= (1 << i)`

#### `getBit(int i, iRegister *r)`
- **Purpose**: Returns the value of the i-th bit
- **Parameters**:
  - `i`: Bit position (0-31)
  - `r`: Pointer to iRegister
- **Returns**: 0 or 1
- **Implementation**: Uses bitwise AND with mask: `(r->content & (1 << i)) ? 1 : 0`

### Bulk Operations

#### `resetAll(iRegister *r)`
- **Purpose**: Sets all bits to 0
- **Implementation**: `r->content = 0`

#### `setAll(iRegister *r)`
- **Purpose**: Sets all bits to 1
- **Implementation**: `r->content = -1` (all bits set in two's complement)

### Nibble Operations

#### `getNibble(int pos, iRegister *r)`
- **Purpose**: Gets a 4-bit nibble value
- **Parameters**:
  - `pos`: Nibble position (1 for lower nibble bits 0-3, 2 for upper nibble bits 4-7)
  - `r`: Pointer to iRegister
- **Implementation**:
  - Lower nibble: `r->content & 0xF`
  - Upper nibble: `(r->content >> 4) & 0xF`

#### `assignNibble(int value, int pos, iRegister *r)`
- **Purpose**: Sets a 4-bit nibble to the specified value
- **Parameters**:
  - `value`: Nibble value (0-15)
  - `pos`: Nibble position (1 or 2)
  - `r`: Pointer to iRegister

### Shift Operations

#### `shiftLeft(int n, iRegister *r)`
- **Purpose**: Performs logical left shift
- **Parameters**:
  - `n`: Number of positions to shift (0-31)
  - `r`: Pointer to iRegister
- **Implementation**: `r->content <<= n`
- **Note**: Fills with zeros from the right, leftmost bits are lost

#### `shiftRight(int n, iRegister *r)`
- **Purpose**: Performs logical right shift
- **Parameters**:
  - `n`: Number of positions to shift (0-31)
  - `r`: Pointer to iRegister
- **Implementation**: Casts to unsigned for logical shift: `(unsigned int)r->content >> n`
- **Note**: Fills with zeros from the left

### Display Function

#### `reg2str(iRegister r)`
- **Purpose**: Converts register content to binary string representation
- **Returns**: Static string with 32 characters ('0' or '1') plus null terminator
- **Implementation**: Iterates through each bit using bit shifting and masking

## UART Communication

The application uses UART for user interaction:

### Key UART Functions

#### `uart_send(unsigned int c)`
- Sends a single character over UART
- Low-level function for character transmission

#### `uart_puts(char *s)`
- Sends a null-terminated string over UART
- Automatically converts `\n` to `\r\n` for proper terminal display
- Built on top of `uart_send()`

#### `uart_getc()`
- Receives a single character from UART
- Blocks until character is available

#### `print2uart(const char* fmt, ...)`
- Printf-like function for formatted output over UART
- Uses variable arguments for flexible formatting

### Helper Function

#### `uart_read_number()`
- Reads a decimal number from UART input
- Echoes characters as they are typed
- Converts string input to integer using `atoi()`

## User Interface Flow

1. **Initialization**:
   - UART initialization and screen clear
   - User name input and greeting

2. **Register Setup**:
   - User enters initial decimal value
   - Display in both binary and decimal formats

3. **Interactive Testing**:
   - Reset bit operation with before/after display
   - Reset all bits demonstration
   - Set bit operation with before/after display
   - Set all bits demonstration
   - Read bit operation with bit value display

4. **Advanced Operations**:
   - Nibble operations (get and set lower/upper nibbles)
   - Shift operations (left and right shifts)

5. **Final Display**:
   - Completion message

## Error Handling

All functions include comprehensive error checking:

- **NULL Pointer Checks**: All functions verify pointer parameters
- **Range Validation**: Bit positions validated (0-31), nibble positions (1-2)
- **Post-condition Verification**: Operations verified after execution
- **Error Reporting**: Detailed error messages sent to `stderr`

## Build Configuration

The Makefile is configured for ARM Cortex-A53 (Raspberry Pi 3):

- **Target Architecture**: ARMv8-A with CRC extensions
- **CPU Tuning**: Cortex-A53 optimized
- **Floating Point**: Software floating point
- **Optimization**: Size optimization (-Os)
- **Standards**: GNU C99

## Dependencies

- Standard C library (stdio.h, string.h, stdlib.h)
- Custom UART library for Raspberry Pi
- GPIO libraries for hardware access
- ARM cross-compilation toolchain

## Memory Safety

- All pointer parameters are validated for NULL
- Array bounds are checked for string operations
- Static allocation used for string representation to avoid memory leaks
- Proper null termination of strings
