# Real-Time Embedded Systems - Assignment 1

This repository contains two projects (a1p1 and a1p2) for the Real-Time Embedded Systems course at Halmstad University, designed to run on Raspberry Pi 3 hardware.

## 📋 Table of Contents

- [Project Overview](#project-overview)
- [Documentation](#documentation)
- [Hardware Requirements](#hardware-requirements)
- [Build Requirements](#build-requirements)
- [Quick Start](#quick-start)
- [Project Structure](#project-structure)
- [Generated Files](#generated-files)
- [Running the Applications](#running-the-applications)

## Project Overview

### a1p1 - Integer Register Manipulation
A comprehensive bare-metal application that demonstrates bit manipulation operations on a 32-bit integer register through an interactive UART interface. Features include individual bit operations, nibble manipulation, shift operations, and binary visualization.

### a1p2 - LED Control
A simple bare-metal application that demonstrates basic GPIO control by continuously blinking an LED at 1-second intervals using hardware timers.

## 📚 Documentation

### 📖 Project-Specific Documentation
- **[A1P1 Complete Documentation](a1p1/README.md)** - Detailed documentation for the integer register manipulation project
  - Function-by-function analysis
  - UART communication system
  - User interface flow
  - Error handling and memory safety
  
- **[A1P2 Complete Documentation](a1p2/README.md)** - Detailed documentation for the LED control project
  - GPIO hardware configuration
  - LED control implementation
  - Platform compatibility
  - Timing and hardware registers

### 🔧 Technical Documentation
- **[Makefile Documentation](MAKEFILE_DOCUMENTATION.md)** - Comprehensive build system analysis
  - Cross-compilation setup
  - Compiler flags explanation
  - Build process flow
  - Platform-specific configurations
  
- **[Library Documentation](LIBRARY_DOCUMENTATION.md)** - Complete library reference
  - Hardware abstraction layers
  - Core library functions
  - System-level components
  - Memory management patterns

## Hardware Requirements

- **Raspberry Pi 3** (RPi3) or **Raspberry Pi 3B+**
- **UART connection** for a1p1 project (115200 baud, 8N1)
- **GPIO access** for LED control in a1p2
- **SD card** for image deployment
- **Power supply** (5V, minimum 2.5A recommended)

## Build Requirements

- **ARM cross-compilation toolchain** (`arm-none-eabi-gcc`)
- **GNU Make** utility
- **Linux/WSL environment** (recommended)
- **Hardware-specific definitions** for RPi3

### Toolchain Installation (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install gcc-arm-none-eabi make
```

## 🚀 Quick Start

### Building Both Projects
```bash
# Build a1p1
cd a1p1
make clean && make all

# Build a1p2  
cd ../a1p2
make clean && make all
```

### Individual Project Builds

#### For a1p1 (Interactive Register Manipulation):
```bash
cd a1p1
make clean
make all
# Generates: a1p1.img
```

#### For a1p2 (LED Blinking):
```bash
cd a1p2
make clean
make all
# Generates: a1p2.img
```

## Project Structure

```
Assignment-1/
├── README.md                          # This file - main documentation
├── MAKEFILE_DOCUMENTATION.md          # Build system documentation  
├── LIBRARY_DOCUMENTATION.md           # Library reference
├── a1p1/                              # Integer register manipulation
│   ├── README.md                      # A1P1 detailed documentation
│   ├── a1p1.c                         # Main application
│   ├── Makefile                       # Build configuration
│   └── lib/                           # Libraries
│       ├── iregister.h/.c             # Register manipulation
│       ├── uart.h/.c                  # UART communication
│       ├── rpi-gpio.h/.c              # GPIO hardware abstraction
│       ├── rpi-base.h                 # Hardware base definitions
│       ├── rpi-systimer.h/.c          # System timer
│       ├── startup.c                  # Boot sequence
│       └── ...                        # Additional hardware libraries
└── a1p2/                              # LED control project
    ├── README.md                      # A1P2 detailed documentation
    ├── a1p2.c                         # Main application
    ├── Makefile                       # Build configuration  
    └── lib/                           # Libraries
        ├── led.h/.c                   # LED control functions
        ├── rpi-gpio.h/.c              # GPIO hardware abstraction
        ├── rpi-systimer.h/.c          # System timer for delays
        └── ...                        # Shared hardware libraries
```

## Generated Files

Each project generates the following files:

### Object Files
- **`*.o`** - Compiled object files for each source file
- **`lib/*.o`** - Library object files

### Executable Files  
- **`*.elf`** - Executable and Linkable Format file with debug symbols
- **`*.img`** - **Raw binary image for Raspberry Pi deployment**

### Build Artifacts
- **`*.map`** - Memory map files (if generated)
- **`*.lst`** - Assembly listings (if requested)

## Running the Applications

### Deployment
1. **Copy the `.img` file** to the boot partition of an SD card
2. **Rename it to `kernel7.img`** (for RPi3)
3. **Insert SD card** into Raspberry Pi 3
4. **Power on** the device

### Expected Behavior

#### A1P1 (Register Manipulation)
- **UART Output**: Interactive menu system at 115200 baud
- **User Input**: Prompts for name, values, and bit positions
- **Display**: Binary representation and decimal values
- **Operations**: Comprehensive bit manipulation testing

#### A1P2 (LED Control)  
- **LED Behavior**: Continuous blinking at 1-second intervals
- **GPIO**: Uses GPIO16 (or platform-specific LED pin)
- **Operation**: Infinite loop until power off

### Connection Requirements

#### For A1P1 (UART Communication)
- **Connect UART adapter** to GPIO14 (TX) and GPIO15 (RX)
- **Terminal settings**: 115200 baud, 8 data bits, no parity, 1 stop bit
- **Terminal software**: PuTTY, minicom, or similar

#### For A1P2 (LED Control)
- **External LED**: Connect to GPIO16 with current-limiting resistor
- **Built-in LED**: May use platform-specific activity LED

## 🔍 Troubleshooting

### Build Issues
- **Missing toolchain**: Install `gcc-arm-none-eabi`
- **Permission errors**: Check file permissions and paths
- **Missing files**: Ensure all library files are present

### Runtime Issues  
- **No UART output**: Check connection and baud rate
- **LED not blinking**: Verify GPIO connections and power
- **Boot failure**: Ensure correct image naming and SD card formatting

## 📞 Support

For detailed information about specific components, refer to the linked documentation files above. Each document provides comprehensive technical details, implementation explanations, and troubleshooting guidance.
