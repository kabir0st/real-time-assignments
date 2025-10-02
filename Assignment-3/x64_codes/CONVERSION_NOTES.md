# Raspberry Pi to x86_64 Linux Conversion Notes

## Summary

Successfully converted the TinyThreads cooperative threading library from Raspberry Pi (ARMv8) to x86_64 Linux for testing and development on WSL2.

## Conversion Date
October 2, 2025

## Key Changes

### 1. Context Switching Mechanism

**Original (ARM):**
- Used `setjmp`/`longjmp` with manual stack pointer manipulation
- Directly modified `jmp_buf` internals to set custom stack

**Converted (x86_64):**
- Uses POSIX `ucontext_t` API (getcontext, setcontext, swapcontext, makecontext)
- Portable and standardized approach
- Proper stack allocation with `uc_stack` structure

**Why?** Manipulating `jmp_buf` internals is non-portable and unreliable across architectures.

### 2. Interrupt Control

**Original (ARM):**
```c
__asm volatile("CPSIE i \n");  // Enable interrupts
__asm volatile("CPSID i \n");  // Disable interrupts
```

**Converted (x86_64):**
```c
sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);  // Enable
sigprocmask(SIG_BLOCK, &signal_mask, &old_mask);  // Disable
```

**Why?** User-space programs can't directly control CPU interrupts. Signal masking provides similar critical section protection.

### 3. Hardware Abstraction Layer

**Removed Dependencies:**
- `rpi-systimer.h` → Replaced with `usleep()`
- `piface.h` → Replaced with `printf()`
- `uart.h` → Replaced with `printf()`
- `rpi-interrupts.h` → Signal handling
- `rpi-gpio.h` → Not needed
- `rpi-armtimer.h` → Not needed

### 4. Stack Size

**Original:** 1024 bytes (sufficient for embedded ARM)
**Converted:** 8192 bytes (safer for x86_64 with larger stack frames)

### 5. Thread Structure

**Changed:**
```c
// Original
jmp_buf context;

// Converted
ucontext_t context;
```

### 6. Thread Wrapper Function

**Added:** A `thread_wrapper()` function to handle thread lifecycle properly with ucontext.

```c
static void thread_wrapper(void) {
    ENABLE();
    current->function(current->arg);
    DISABLE();
    enqueue(current, &freeQ);
    current = NULL;
    // Handle thread completion
}
```

### 7. Compiler Flags

**Original (ARM cross-compiler):**
```makefile
CC = arm-none-eabi-gcc
CFLAGS = -march=armv8-a+crc -mtune=cortex-a53 -mfpu=vfp -mfloat-abi=soft
```

**Converted (native GCC):**
```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -g -Ilib
```

## Files Modified/Created

### Modified from a3p1:
1. **main.c**
   - Removed: piface_init(), piface_clear(), piface_puts(), RPI_WaitMicroSeconds()
   - Added: printf() for console output, usleep() for delays
   - Added: Startup banner and better formatting

2. **lib/tinythreads.c**
   - Complete rewrite of context switching mechanism
   - Changed from setjmp/longjmp to ucontext API
   - Added signal-based interrupt simulation
   - Updated print functions to use printf instead of UART/PiFace

3. **Makefile**
   - Simplified for native Linux compilation
   - Removed ARM-specific linker scripts and startup code
   - Added helpful targets (run, test, info, help)

### Unchanged (reused):
1. **lib/expstruct.c** - Already portable
2. **lib/expstruct.h** - Already portable
3. **lib/tinythreads.h** - API remains the same

### Created:
1. **README.md** - Comprehensive documentation
2. **QUICKSTART.md** - Quick start guide
3. **CONVERSION_NOTES.md** - This file

## Testing Results

✅ **Build:** Compiles cleanly with only minor warnings for stub functions
✅ **Execution:** Threads cooperatively yield correctly
✅ **Output:** Interleaved output from multiple threads as expected
✅ **Stability:** No crashes or segmentation faults
✅ **Portability:** Works on WSL2/Linux x86_64

## Performance Characteristics

- Thread switching: ~microseconds (ucontext overhead)
- Memory per thread: ~8KB stack + ~100 bytes metadata
- Max threads: 5 (configurable via NTHREADS)
- Scheduling: Cooperative (non-preemptive)

## Limitations

1. **Not real-time**: Linux is not a real-time OS, scheduling is best-effort
2. **User-space only**: Cannot test real interrupt handling
3. **No hardware I/O**: Cannot test GPIO, timers, or other peripherals
4. **Signal blocking overhead**: More overhead than actual interrupt disable
5. **Cooperative only**: Threads must voluntarily yield (no preemption)

## Use Cases

✅ **Perfect for:**
- Algorithm development and testing
- Debugging threading logic
- Understanding cooperative multithreading
- Prototyping before deploying to Raspberry Pi
- Teaching threading concepts

❌ **Not suitable for:**
- Real-time performance testing
- Hardware interrupt testing
- GPIO/peripheral interaction testing
- Preemptive scheduling validation

## Migration Path

To port code from x64 back to Raspberry Pi:
1. Replace printf() with PUTTOLDC() or piface_puts()
2. Replace usleep() with RPI_WaitMicroSeconds()
3. Add piface_init() at startup
4. Recompile with ARM toolchain
5. Test on actual hardware

## Technical Details

### ucontext API Usage

```c
// Create context
getcontext(&newp->context);

// Set stack
newp->context.uc_stack.ss_sp = newp->stack;
newp->context.uc_stack.ss_size = STACKSIZE;

// Set entry point
makecontext(&newp->context, thread_wrapper, 0);

// Switch contexts
swapcontext(&old->context, &next->context);
```

### Signal Masking

```c
// Initialize
sigfillset(&signal_mask);  // Block all signals

// Disable "interrupts"
sigprocmask(SIG_BLOCK, &signal_mask, &old_mask);

// Enable "interrupts"
sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
```

## Future Enhancements

Possible improvements for Assignment 4:
- Implement mutex (lock/unlock)
- Add deadline-based scheduling
- Implement Round-Robin scheduler
- Add Rate Monotonic scheduler
- Add Earliest Deadline First scheduler
- Add timer-based preemption using SIGALRM

## References

- POSIX ucontext: `man 3 makecontext`
- Signal handling: `man 2 sigprocmask`
- Original TinyThreads: Johan Nordlander & Fredrik Bengtsson (LTU)
- ARM port: Wagner de Morais & Hazem Ali

## Build Environment

- **OS:** Linux (WSL2 on Windows)
- **Architecture:** x86_64
- **Compiler:** GCC (GNU Compiler Collection)
- **C Standard:** GNU99
- **Required Libraries:** POSIX (ucontext, signals)

## Verification

To verify the conversion works:
```bash
cd x64_codes
make clean && make
./main
```

You should see interleaved output from threads T0 and T1.
Press Ctrl+C to stop.

---

**Conversion Author:** AI Assistant
**Original Code:** DT8025 - Real-Time Systems Course
**Purpose:** Enable development and testing on x86_64 Linux/WSL2

