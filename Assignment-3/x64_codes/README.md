# TinyThreads x86_64 Linux Port

This is a port of the TinyThreads library from Raspberry Pi (ARM) to x86_64 Linux for testing and development on WSL2/Linux systems.

## Overview

The original code was designed for Raspberry Pi with ARM-specific features:
- ARM assembly instructions for interrupt control (CPSIE/CPSID)
- Raspberry Pi hardware interfaces (PiFace, UART, System Timer)
- ARM-specific stack pointer manipulation

This port adapts the code to run on x86_64 Linux by:
- Replacing ARM assembly with POSIX signal blocking for "interrupt" simulation
- Replacing hardware-specific functions with standard C library equivalents
- Adapting stack pointer manipulation for x86_64 architecture
- Using `printf()` instead of PiFace display
- Using `usleep()` instead of `RPI_WaitMicroSeconds()`

## Structure

```
x64_codes/
├── main.c                    # Main application with test threads
├── Makefile                  # Build configuration
├── lib/
│   ├── tinythreads.c         # Cooperative threading library
│   ├── tinythreads.h         # Threading API header
│   ├── expstruct.c           # Exponential calculation utilities
│   └── expstruct.h           # Exponential utilities header
└── README.md                 # This file
```

## Features

The threading library implements:
- **Cooperative multithreading** using `setjmp`/`longjmp`
- **Thread spawning** with `spawn()`
- **Thread yielding** with `yield()`
- **Thread queues** (ready, free, done)
- **Context switching** between threads

### Test Threads

The main.c includes two sample threads:
1. **computePower(seg)** - Computes and displays squares of integers
2. **computePrimes(seg)** - Finds and displays prime numbers

These threads cooperatively yield control to each other, demonstrating the threading system.

## Building

### Prerequisites

- GCC compiler
- Make
- Linux or WSL2 environment

### Build Commands

```bash
# Build the executable
make

# Build and run
make run

# Clean build artifacts
make clean

# Rebuild from scratch
make rebuild

# Show build configuration
make info

# Show help
make help
```

## Running

```bash
# After building
./main
```

The program will:
1. Display a startup message
2. Spawn the computePower thread
3. Run the computePrimes thread in the main context
4. Both threads will cooperatively print their results

**Note:** The threads run indefinitely. Press `Ctrl+C` to stop.

## Key Differences from Raspberry Pi Version

### 1. Interrupt Control
**Raspberry Pi (ARM):**
```c
__asm volatile("CPSIE i \n");  // Enable interrupts
__asm volatile("CPSID i \n");  // Disable interrupts
```

**x86_64 Linux:**
```c
sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);  // Enable
sigprocmask(SIG_BLOCK, &signal_mask, NULL);    // Disable
```

### 2. Stack Setup
**Raspberry Pi (ARM):**
```c
*((unsigned int *)(buf)+8) = (unsigned int)(a) + STACKSIZE - 4;
```

**x86_64 Linux:**
```c
((long long int*)(buf))[6] = (long long int)stack_top;
```

### 3. Hardware Functions
| Raspberry Pi | x86_64 Linux |
|--------------|--------------|
| `piface_init()`, `piface_puts()` | Removed (not needed) |
| `RPI_WaitMicroSeconds()` | `usleep()` |
| `print2uart()` | `printf()` |
| `PUTTOLDC()` macro | `printf()` |

## Limitations

- **No hardware interaction** - PiFace, GPIO, and other hardware features are not available
- **Simulated interrupt control** - Uses signal masking instead of real interrupt disable/enable
- **User-space only** - Cannot test real-time scheduling or hardware interrupt handling
- **Scheduler functions** - Advanced scheduling features (RR, RM, EDF) are stubbed out for Assignment 4

## Testing and Tinkering

This port is ideal for:
- Understanding the threading algorithm without hardware
- Debugging and testing thread logic
- Developing and testing new thread functions
- Learning cooperative multithreading concepts

You can modify `main.c` to add your own thread functions or modify existing ones.

## Example: Adding a New Thread

```c
void myThread(int seg) {
    for(int i = 0; ; i++) {
        printf("T%d: Count %d\n", seg, i);
        usleep(500000);
        yield();
    }
}

int main() {
    printf("Starting threads...\n");
    spawn(myThread, 2);      // Spawn your thread
    spawn(computePower, 0);  // Spawn power thread
    computePrimes(1);        // Run primes in main
    return 0;
}
```

## Architecture Notes

### Thread Structure
Each thread has:
- Unique ID
- Function pointer and argument
- Execution stack (1024 bytes)
- Context (jmp_buf for register state)
- Period and deadline fields (for future scheduling)

### Thread Queues
- **freeQ** - Available thread blocks
- **readyQ** - Threads ready to execute
- **doneQ** - Completed threads (for Assignment 4)

### Context Switching
Uses `setjmp`/`longjmp` to save/restore CPU registers and switch between thread contexts.

## Credits

- Original code: Johan Nordlander and Fredrik Bengtsson (LTU)
- ARM extensions: Wagner de Morais and Hazem Ali
- x86_64 port: Adapted for WSL2/Linux testing

## License

See LICENSE file in parent directory.

