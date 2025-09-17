# Real-Time Systems Course

This repository contains assignments and projects for the Real-Time Systems course.

## Overview

This course covers fundamental concepts in real-time systems including:
- Real-time system design principles
- Hardware register manipulation and bit-level operations
- System programming and low-level interfaces
- Performance analysis and optimization
- Embedded systems programming

## Assignments

### Assignment 1: Hardware Register Simulation

**Assignment 1** focuses on implementing and understanding low-level hardware register operations through C programming.

#### Parts:

- **[a1p1 - iRegister Library](Assignment-1/a1p1/README.md)**: A comprehensive C library for 32-bit hardware register simulation with bit manipulation capabilities
  - Individual bit operations (set, reset, get)
  - Bulk operations (set all, reset all)
  - Nibble-level operations
  - Bit shifting operations
  - Complete testing framework

- **a1p2**: *Documentation pending*

## Getting Started

### Prerequisites

- GCC compiler with C99 support
- Make build system
- Linux/Unix environment (tested on WSL2)

### Building and Running

Each assignment part contains its own build system. Navigate to the specific directory and follow the instructions in the respective README files.

For Assignment 1 Part 1:
```bash
cd Assignment-1/a1p1
make
./main
```

## Repository Structure

```
real-time/
├── README.md                   # This file
└── Assignment-1/
    ├── a1p1/                   # Hardware register library
    │   ├── README.md           # Comprehensive documentation
    │   ├── main.c              # Testing framework
    │   ├── Makefile            # Build system
    │   └── lib/
    │       ├── iregister.h     # Header file
    │       └── iregister.c     # Implementation
    ├── a1p2/                   # Assignment 1 Part 2
    └── Assignment1.pdf         # Assignment specification
```

## Course Information

- **Course**: Real-Time Systems
- **Institution**: University
- **Academic Year**: 2025

## Documentation

Detailed documentation for each assignment part is available in the respective subdirectories:

- [Assignment 1 Part 1 Documentation](Assignment-1/a1p1/README.md) - Complete implementation guide for the iRegister library

## Contributing

This is a course assignment repository. Each assignment follows academic integrity guidelines.

---

*For detailed implementation information, build instructions, and usage examples, please refer to the individual assignment documentation.*
