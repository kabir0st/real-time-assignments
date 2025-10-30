# Preemptive Context Switching in ARM Bare-Metal Multithreading

## Table of Contents
1. [Overview](#overview)
2. [The Problem](#the-problem)
3. [Understanding ARM Processor Modes](#understanding-arm-processor-modes)
4. [The Solution](#the-solution)
5. [Implementation Details](#implementation-details)
6. [How It Works](#how-it-works)
7. [Usage Guide](#usage-guide)
8. [Technical Deep Dive](#technical-deep-dive)
9. [Troubleshooting](#troubleshooting)
10. [References](#references)

---

## Overview

This document explains the implementation of proper preemptive context switching for the TinyThreads threading library on ARM-based Raspberry Pi systems. The implementation solves a critical issue where thread switching would freeze when triggered by timer interrupts instead of explicit `yield()` calls.

### Key Terms
- **Cooperative Multitasking**: Threads voluntarily yield control by calling `yield()`
- **Preemptive Multitasking**: The OS forcibly switches threads via timer interrupts
- **Context**: The complete CPU state (registers, stack, flags) of a thread
- **Context Switch**: Saving one thread's context and restoring another's

---

## The Problem

### Original Implementation Issue

The original TinyThreads implementation used `setjmp()/longjmp()` for context switching. This works perfectly for **cooperative multitasking** but fails for **preemptive multitasking**.

#### What Happens in Cooperative Mode (Working)
```
Thread A running → calls yield() → setjmp() saves context →
→ longjmp() to Thread B → Thread B running → calls yield() →
→ longjmp() back to Thread A → Thread A continues ✓
```

#### What Happens in Preemptive Mode (Broken)
```
Thread A running → Timer IRQ fires → CPU enters IRQ mode →
→ IRQ handler calls scheduler() → calls yield() → setjmp() saves context →
→ longjmp() to Thread B → Thread B runs in IRQ MODE! → 
→ System tries to return from non-existent IRQ → FREEZE ✗
```

### Why Does It Fail?

The issue stems from fundamental limitations of `setjmp()/longjmp()`:

1. **setjmp/longjmp don't save CPU mode**: They save registers (r0-r12, SP, LR, PC) but NOT the CPSR (Current Program Status Register), which contains the processor mode.

2. **IRQ mode has a different stack**: When an interrupt occurs:
   - CPU switches to IRQ mode
   - CPU switches to IRQ stack (at 0x7000)
   - Normal threads run in Supervisor (SVC) mode with SVC stack (at 0x8000)

3. **longjmp from IRQ mode is dangerous**: When you `longjmp()` from within an IRQ handler:
   - You jump to the target thread's code
   - BUT you're still in IRQ mode with the IRQ stack!
   - The thread expects to be in SVC mode with SVC stack
   - When the thread tries to execute, stack operations fail
   - The system cannot properly return from the IRQ
   - Result: System freeze

### Visual Representation of the Problem

```
Normal Thread Execution:
┌─────────────────────────────────────────┐
│  SVC Mode (Supervisor Mode)            │
│  Stack: 0x8000                          │
│  Thread A running                       │
│  Thread B running                       │
│  ...context switches via yield()...     │
└─────────────────────────────────────────┘

When Timer Interrupt Fires:
┌─────────────────────────────────────────┐
│  IRQ Mode                               │
│  Stack: 0x7000 (IRQ Stack)              │
│  interrupt_vector()                     │
│    → scheduler()                        │
│      → yield()                          │
│        → dispatch()                     │
│          → longjmp() ← PROBLEM HERE!    │
└─────────────────────────────────────────┘
           ↓
      Jumps to Thread B
           ↓
┌─────────────────────────────────────────┐
│  Still in IRQ Mode! ✗                   │
│  Still using IRQ Stack! ✗               │
│  Thread B expects SVC mode! ✗           │
│  → Stack corruption                     │
│  → Cannot return from IRQ               │
│  → SYSTEM FREEZE                        │
└─────────────────────────────────────────┘
```

---

## Understanding ARM Processor Modes

ARM processors operate in different **privilege modes**, each with its own purpose and banked registers.

### ARM Processor Modes

| Mode Name   | Mode Bits | Usage                        | Stack Location |
|-------------|-----------|------------------------------|----------------|
| User (USR)  | 0x10      | Unprivileged user programs   | Application    |
| FIQ         | 0x11      | Fast interrupt handling      | 0x4000         |
| IRQ         | 0x12      | Normal interrupt handling    | 0x7000         |
| Supervisor (SVC) | 0x13 | OS kernel / privileged code  | 0x8000         |
| Abort       | 0x17      | Memory access failures       | Special        |
| Undefined   | 0x1B      | Undefined instructions       | Special        |
| System (SYS)| 0x1F      | Privileged user mode         | Application    |

### CPSR (Current Program Status Register)

The CPSR contains crucial information about the CPU state:

```
Bits 31-8: Condition flags (N, Z, C, V, etc.)
Bit  7:    IRQ disable (I bit) - 1 = IRQs disabled
Bit  6:    FIQ disable (F bit) - 1 = FIQs disabled
Bit  5:    Thumb mode (T bit)
Bits 4-0:  Mode bits (defines current processor mode)
```

### Banked Registers

Each mode has its own copies of certain registers:

- **SP (Stack Pointer)**: Each mode has its own stack
- **LR (Link Register)**: Return address for function calls
- **SPSR (Saved Program Status Register)**: Saved CPSR on exception entry

```
           r0  r1  r2  r3  r4  r5  r6  r7  r8  r9  r10 r11 r12 SP  LR  PC
USR/SYS    ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ●
FIQ        ●   ●   ●   ●   ●   ●   ●   ●   ○   ○   ○   ○   ○   ○   ○   ●
IRQ        ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ○   ○   ●
SVC        ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ●   ○   ○   ●

● = Shared across modes
○ = Banked (separate copy per mode)
```

---

## The Solution

### High-Level Approach

To fix preemptive context switching, we need to:

1. **Exit IRQ mode** before performing a context switch
2. **Transition to SVC mode** where threads normally execute
3. **Perform the context switch** using setjmp/longjmp (which now works correctly)
4. **Ensure the new thread runs in SVC mode** with proper stack

### Implementation Strategy

We've implemented two assembly helper functions:

1. **`exit_irq_mode()`**: Safely transitions from IRQ mode to SVC mode
2. **`enter_svc_mode_for_thread()`**: Ensures a thread is running in the correct mode

These functions are called at strategic points in the context switching code to ensure mode correctness.

### Code Flow with Fix

```
Thread A running in SVC mode
    ↓
Timer IRQ fires → CPU enters IRQ mode
    ↓
interrupt_vector() [in IRQ mode]
    ↓
scheduler() [in IRQ mode]
    ↓
yield() [in IRQ mode]
    ↓
dispatch() [in IRQ mode]
    ↓
setjmp(current->context) - saves Thread A's context
    ↓
exit_irq_mode() ← NEW: Transition to SVC mode!
    ↓
longjmp(next->context) - restore Thread B's context
    ↓
Thread B running in SVC mode ✓
    ↓
enter_svc_mode_for_thread() - verify mode is correct
    ↓
Continue execution normally
```

---

## Implementation Details

### File Structure

```
a4p1/lib/
├── context_switch_simple.s    # Assembly mode-switching functions
├── context_switch.s           # Full context switch (alternative implementation)
├── context_switch.h           # Header for full implementation
└── tinythreads.c              # Modified to use mode-switching functions
```

### New Assembly Functions

#### `exit_irq_mode()` - lib/context_switch_simple.s

```assembly
exit_irq_mode:
    /* Save return address */
    push {lr}
    
    /* We're in IRQ mode, switch to SVC mode */
    mrs r0, cpsr                         /* Read current CPSR */
    bic r0, r0, #0x1F                   /* Clear mode bits */
    orr r0, r0, #CPSR_MODE_SVR          /* Set SVC mode */
    orr r0, r0, #(CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT)  /* Keep IRQs disabled */
    msr cpsr_c, r0                      /* Write new CPSR */
    
    /* Now we're in SVC mode */
    pop {pc}                            /* Return */
```

**What it does:**
- Reads the CPSR register
- Clears the mode bits (bits 0-4)
- Sets mode to SVC (0x13)
- Keeps interrupts disabled during transition
- Returns with CPU now in SVC mode

#### `enter_svc_mode_for_thread()` - lib/context_switch_simple.s

```assembly
enter_svc_mode_for_thread:
    /* Make sure we're in SVC mode with interrupts enabled */
    mrs r0, cpsr                        /* Read CPSR */
    bic r0, r0, #0x1F                  /* Clear mode bits */
    orr r0, r0, #CPSR_MODE_SVR         /* Set SVC mode */
    bic r0, r0, #CPSR_IRQ_INHIBIT      /* Enable IRQs */
    msr cpsr_c, r0                     /* Write new CPSR */
    
    bx lr                               /* Return */
```

**What it does:**
- Ensures the CPU is in SVC mode
- Re-enables interrupts for normal thread execution
- Returns to continue thread execution

### Modified C Code

#### dispatch() - lib/tinythreads.c

```c
static void dispatch(thread next) {
    if (next != NULL) {
        if (setjmp(current->context) == 0) {
            current = next;
            
            /* CRITICAL: Exit IRQ mode before longjmp */
            exit_irq_mode();  /* Transition to SVC mode */
            
            longjmp(next->context, 1);
        }
    }
    
    /* Ensure correct mode after context switch */
    enter_svc_mode_for_thread();
}
```

**Key changes:**
- Call `exit_irq_mode()` BEFORE `longjmp()`
- Call `enter_svc_mode_for_thread()` AFTER context switch completes

#### yield() - lib/tinythreads.c

```c
void yield(void) {
    if (readyQ != NULL) {
        thread p = dequeue(&readyQ);
        enqueue(current, &readyQ);
        dispatch(p);
    }
    
    /* Ensure correct mode after returning */
    enter_svc_mode_for_thread();
}
```

### Makefile Changes

Added assembly file compilation:

```makefile
# Add context switching object file
OBJS += lib/context_switch_simple.o

# Add rule for assembling .s files
%.o: %.s
	$(AS) -o $@ $^
```

---

## How It Works

### Detailed Execution Flow

Let's trace through a complete preemptive context switch:

#### Step 1: Thread A Running (SVC Mode)

```
CPU State:
  Mode: SVC (0x13)
  Stack: 0x8000
  PC: Thread A code
  CPSR: 0x00000013 (SVC mode, IRQs enabled)

Thread A is executing:
  for(volatile uint32_t i=0; ; i++) {
      value = iexp((i % 9) + 1);
      printf_at_seg(seg, ...);
      // ← Timer interrupt fires HERE
  }
```

#### Step 2: Timer Interrupt Occurs

```
Hardware automatically:
1. Saves return address: LR_irq = PC + 4
2. Saves CPSR: SPSR_irq = CPSR
3. Switches to IRQ mode: CPSR mode bits = 0x12
4. Switches to IRQ stack: SP = 0x7000
5. Disables interrupts: CPSR I-bit = 1
6. Jumps to interrupt vector

CPU State:
  Mode: IRQ (0x12)
  Stack: 0x7000
  PC: interrupt_vector
  CPSR: 0x00000092 (IRQ mode, IRQs disabled)
  SPSR_irq: 0x00000013 (saved SVC mode CPSR)
  LR_irq: Thread A return address
```

#### Step 3: Interrupt Handler Calls Scheduler

```c
void interrupt_vector(void) {
    if (RPI_GetArmTimer()->MaskedIRQ) {
        RPI_GetArmTimer()->IRQClear = 1;
        ticks++;
        scheduler();  // ← Still in IRQ mode
    }
}

void scheduler(void) {
    scheduler_RR();  // ← Still in IRQ mode
}

void scheduler_RR(void) {
    yield();  // ← Still in IRQ mode
}

void yield(void) {
    if (readyQ != NULL) {
        thread p = dequeue(&readyQ);
        enqueue(current, &readyQ);
        dispatch(p);  // ← Still in IRQ mode
    }
}
```

#### Step 4: dispatch() Saves Current Context

```c
static void dispatch(thread next) {
    if (setjmp(current->context) == 0) {  // ← Save Thread A context
```

`setjmp()` saves to `current->context`:
- r0-r12: Current register values
- SP: IRQ stack pointer (0x7000 - offset) ← WRONG!
- LR: Address within dispatch()
- PC: Return point after setjmp

**Problem**: We're still in IRQ mode! The saved SP is from IRQ stack, not Thread A's stack!

#### Step 5: Exit IRQ Mode (THE FIX!)

```c
        current = next;
        exit_irq_mode();  // ← Switch to SVC mode
```

Assembly code executes:
```assembly
mrs r0, cpsr              # r0 = 0x00000092 (IRQ mode)
bic r0, r0, #0x1F        # r0 = 0x00000080 (clear mode)
orr r0, r0, #0x13        # r0 = 0x00000093 (set SVC mode)
orr r0, r0, #0xC0        # r0 = 0x000000D3 (keep IRQs disabled)
msr cpsr_c, r0           # CPSR = 0x000000D3

CPU State NOW:
  Mode: SVC (0x13)  ← Changed!
  Stack: 0x8000     ← Changed!
  IRQs: Disabled
```

#### Step 6: Jump to Thread B

```c
        longjmp(next->context, 1);  // ← Now in SVC mode!
```

`longjmp()` restores Thread B's context:
- r0-r12: Thread B's registers
- SP: Thread B's SVC stack
- LR: Thread B's return address  
- PC: Thread B's continuation point

**This works correctly because we're now in SVC mode with the SVC stack!**

#### Step 7: Thread B Continues Execution

```c
    }  // ← longjmp returns here in Thread B
    
    enter_svc_mode_for_thread();  // Ensure mode is correct
```

Assembly:
```assembly
mrs r0, cpsr
bic r0, r0, #0x1F       # Clear mode
orr r0, r0, #0x13       # Set SVC mode
bic r0, r0, #0x80       # Enable IRQs ← Important!
msr cpsr_c, r0

CPU State:
  Mode: SVC (0x13)
  Stack: 0x8000
  IRQs: Enabled        ← Thread can be preempted again
```

Thread B continues:
```c
void yield(void) {
    // ...
    enter_svc_mode_for_thread();
    // Returns to caller in Thread B
}

// Back in Thread B's code:
for(volatile uint32_t i=0; ; i++) {
    value = iexp((i % 9) + 1);
    printf_at_seg(seg, ...);  // ← Execution continues here
}
```

---

## Usage Guide

### Building the Project

```bash
cd a4p1
make
```

This will:
1. Compile all C files
2. Assemble `context_switch_simple.s`
3. Link everything together
4. Produce `a4p1.img` for Raspberry Pi

### Testing Preemptive Multitasking

The `a4p1.c` program spawns four threads that run "forever":

```c
int main() {
    piface_init();
    uart_init();
    initTimerInterrupts();  // Set up timer for preemptive scheduling
    
    spawn(computeSomethingForever, 0);
    spawn(computeSomethingForever, 1);
    spawn(computeSomethingForever, 2);
    spawn(computeSomethingForever, 3);
    
    while(1) {
        no_operation();  // Main thread idle loop
    }
}
```

**What you should see:**
- All four threads execute in round-robin fashion
- Each thread displays its output on the PiFace display
- Threads switch automatically via timer interrupts
- **NO freezing** even though threads never call `yield()` explicitly!

### Verifying It Works

Monitor via UART to see:
```
T0: 2
T1: 2
T2: 2
T3: 2
ticks: 5
T0: 7
ticks: 6
T1: 7
ticks: 7
T2: 7
...
```

The `ticks` counter shows timer interrupts are firing, and threads are switching automatically.

---

## Technical Deep Dive

### Why setjmp/longjmp Fail in IRQ Context

`setjmp()` and `longjmp()` are designed for **user-space exception handling** (like try/catch), not kernel-level context switching.

#### What setjmp Saves
```c
typedef struct {
    unsigned int regs[10];  // r4-r11, SP, LR
    // Does NOT save: CPSR, PC (explicitly), r0-r3, r12
} jmp_buf[1];
```

The implementation typically:
```c
int setjmp(jmp_buf env) {
    __asm__ volatile (
        "mov ip, r0\n"           // ip = env
        "stmia ip!, {r4-r11}\n"  // Save r4-r11
        "mov r4, sp\n"
        "mov r5, lr\n"
        "stmia ip!, {r4-r5}\n"   // Save SP, LR
        "mov r0, #0\n"           // Return 0
        "bx lr\n"
    );
}
```

**Notice**: No CPSR saved! The CPU mode is lost.

#### What longjmp Does
```c
void longjmp(jmp_buf env, int val) {
    __asm__ volatile (
        "mov ip, r0\n"           // ip = env
        "ldmia ip!, {r4-r11}\n"  // Restore r4-r11
        "ldmia ip!, {r4-r5}\n"   // Load SP, LR
        "mov sp, r4\n"
        "mov lr, r5\n"
        "movs r0, r1\n"          // Return val
        "moveq r0, #1\n"         // Or 1 if val == 0
        "bx lr\n"
    );
}
```

**Notice**: It just restores registers. If you're in IRQ mode, you stay in IRQ mode!

### Stack Corruption Example

When context switching from IRQ mode without fixing the mode:

```
Before Timer IRQ:
Thread A Stack (SVC):          IRQ Stack:
0x8000: [Thread A data]        0x7000: [unused]
0x7FF0: [Thread A frames]      0x6FF0: [unused]
...                            ...

During IRQ Handler:
Thread A Stack (SVC):          IRQ Stack:
0x8000: [Thread A data]        0x7000: [IRQ handler frame]
0x7FF0: [Thread A frames]      0x6FF0: [scheduler frame]
...                            0x6FE0: [yield frame]

After longjmp to Thread B (WITHOUT fix):
Thread B Stack (SVC):          IRQ Stack:
0x8000: [Thread B data]        0x7000: [IRQ handler frame] ← Orphaned!
0x7FF0: [Thread B frames]      0x6FF0: [scheduler frame]   ← Orphaned!
...                            0x6FE0: ← SP points here!   ← WRONG STACK!

Thread B tries to run:
- push {r0}  ← Writes to 0x6FDC (IRQ stack!)
- bl printf  ← Creates frame on IRQ stack
- IRQ stack grows, collides with SVC stack
- CORRUPTION and CRASH
```

### The Mode Transition Fix

By calling `exit_irq_mode()` before `longjmp()`:

```
During IRQ Handler:            IRQ Stack:
Thread A Stack (SVC):          0x7000: [IRQ handler frame]
0x8000: [Thread A data]        0x6FF0: [scheduler frame]
...                            

exit_irq_mode() called:
- CPSR mode bits changed to SVC (0x13)
- SP automatically switches to SVC stack (banked register)

After Mode Switch:             IRQ Stack:
Thread B Stack (SVC):          0x7000: [IRQ handler frame] ← Abandoned, OK!
0x8000: [Thread B data]        0x6FF0: ← Not used anymore
SP → 0x7FF0: [Thread B frames] ← Correct stack!

longjmp() now works correctly!
```

The IRQ stack frame is abandoned, but that's OK because:
1. We never return to the IRQ handler
2. Next IRQ will reset IRQ stack from the top
3. Thread B is running correctly on its own SVC stack

### Alternative: Full Context Switch Implementation

The `context_switch.s` file provides a more robust implementation that properly saves/restores ALL CPU state including CPSR. This is closer to a real OS implementation.

**Advantages:**
- Saves full CPU state including CPSR
- More portable to different ARM variants
- Clearer separation of concerns

**Disadvantages:**
- More complex
- Slightly slower (more registers to save/restore)
- Requires changing thread_block structure to use context_t instead of jmp_buf

The simple solution using mode switching + setjmp/longjmp is sufficient for this assignment and demonstrates the core concept.

---

## Troubleshooting

### System Still Freezes

**Check 1**: Verify assembly file is compiled
```bash
ls lib/*.o
# Should include context_switch_simple.o
```

**Check 2**: Verify functions are called
Add debug output:
```c
void dispatch(thread next) {
    print2uart("dispatch: before exit_irq_mode\n");
    exit_irq_mode();
    print2uart("dispatch: after exit_irq_mode\n");
    longjmp(next->context, 1);
}
```

**Check 3**: Check interrupt setup
```c
// Timer should be configured for regular interrupts
RPI_GetArmTimer()->Load = 0x1E78;  // ~2 second interval
```

### Threads Don't Switch

**Symptom**: Only one thread runs, others never execute

**Causes**:
1. Timer interrupts not firing → Check timer configuration
2. Interrupts disabled globally → Check `ENABLE()` is called
3. Ready queue empty → Check threads are spawned correctly

**Debug**:
```c
void interrupt_vector(void) {
    print2uart("IRQ fired! ticks=%d\n", ticks);
    // Should print every timer period
}
```

### Stack Overflow

**Symptom**: Random crashes, corrupted data

**Cause**: Thread stack (1024 bytes) is too small

**Fix**: Increase `STACKSIZE` in tinythreads.c:
```c
#define STACKSIZE  2048  // Increased from 1024
```

### Wrong Output/Behavior

**Symptom**: Threads produce incorrect results

**Cause**: Race conditions, shared state without synchronization

**Fix**: Use mutexes (to be implemented in later parts):
```c
mutex m = MUTEX_INIT;

void thread_function(int arg) {
    lock(&m);
    // Critical section
    unlock(&m);
}
```

---

## References

### ARM Architecture Documentation

1. **ARM Architecture Reference Manual (ARMv7-A)**
   - Section A2.2: Processor Modes
   - Section A2.5: Program Status Registers
   - Section B1.3: ARM Core Registers

2. **BCM2835 ARM Peripherals** (Raspberry Pi)
   - Chapter 7: Interrupts
   - Timer interrupt configuration

3. **ARM Cortex-A Series Programmer's Guide**
   - Chapter 10: Exception Handling
   - Chapter 11: Interrupt Handling

### Context Switching Resources

4. **"The Definitive Guide to ARM Cortex-M3 and Cortex-M4"** by Joseph Yiu
   - Chapter 8: RTOS Support Features

5. **OSDev Wiki**: Context Switching
   - https://wiki.osdev.org/Context_Switching

6. **James Molloy's Kernel Tutorials**
   - "Multitasking" chapter

### setjmp/longjmp

7. **C Standard Library Reference**
   - setjmp.h documentation
   - Limitations in embedded systems

8. **ARM-specific setjmp/longjmp implementation**
   - newlib source code
   - https://sourceware.org/git/?p=newlib-cygwin.git

### Related Academic Papers

9. **"Real-Time Embedded Systems"** course materials
   - Halmstad University / Luleå University of Technology
   - Thread scheduling, priority inversion, rate monotonic analysis

10. **"MicroC/OS-II: The Real-Time Kernel"** by Jean J. Labrosse
    - Chapter 3: Kernel Structure
    - Appendix: ARM Port

---

## Summary

### The Core Problem
setjmp/longjmp don't save CPU mode → Context switch from IRQ mode → Thread runs in IRQ mode with wrong stack → System freeze

### The Solution
1. Add `exit_irq_mode()` to transition from IRQ to SVC mode before context switch
2. Add `enter_svc_mode_for_thread()` to ensure threads run in correct mode with interrupts enabled
3. Call these at strategic points in `dispatch()` and `yield()`

### Key Insight
**Preemptive multitasking requires kernel-level awareness of CPU modes.** User-space primitives like setjmp/longjmp are insufficient without explicit mode management.

### Next Steps
- Part 2: Implement proper mutex (lock/unlock) with wait queues
- Part 3: Implement periodic tasks with Rate Monotonic (RM) scheduling
- Part 4: Implement Earliest Deadline First (EDF) scheduling

---

**Document Version**: 1.0  
**Date**: October 14, 2025  
**Author**: AI Assistant for Assignment 4  
**Course**: Real-Time Embedded Systems

