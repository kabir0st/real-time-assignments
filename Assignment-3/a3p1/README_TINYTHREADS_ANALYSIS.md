# TinyThreads Analysis - Complete Documentation Package

## 📚 Overview

This documentation package provides a **comprehensive analysis** of the TinyThreads cooperative multithreading system used in Assignment 3, Part 1. The system demonstrates how lightweight threading can be implemented on embedded ARMv8 systems (Raspberry Pi 3) using cooperative scheduling.

---

## 📖 Documentation Files

### 1. **TINYTHREADS_DETAILED_REPORT.md** (Main Report)
   - **Purpose**: Comprehensive technical report covering all aspects of TinyThreads
   - **Contents**:
     - Executive Summary
     - System Architecture
     - Data Structures (thread_block, queues, mutex)
     - Core Mechanisms (context switching, spawning, yielding)
     - Thread Lifecycle and State Transitions
     - Concurrency Management
     - Step-by-Step Execution Flow
     - Performance Analysis
     - Implementation Limitations
   
   📌 **Start here** for a thorough understanding of the entire system.

### 2. **TINYTHREADS_DIAGRAMS.md** (Visual Diagrams)
   - **Purpose**: Interactive Mermaid diagrams for visual learners
   - **Contents**:
     - System Architecture (Graph)
     - Thread State Machine
     - Context Switch Sequence Diagrams
     - spawn() and yield() Flowcharts
     - Queue Operations
     - Memory Layout
     - Execution Timeline (Gantt Chart)
     - Concurrency Model
     - Round-Robin Scheduling
     - Complete System Interaction
   
   📌 **View these** in GitHub, GitLab, or [Mermaid Live Editor](https://mermaid.live)

### 3. **TINYTHREADS_ANIMATED_WALKTHROUGH.md** (Step-by-Step Animation)
   - **Purpose**: Frame-by-frame ASCII animation of system execution
   - **Contents**:
     - System Initialization (9 frames)
     - First Context Switch (8 frames)
     - Round-Robin Steady State
     - Queue State Transitions (6 snapshots)
     - Timeline Visualization
     - Cooperative Scheduling Explanation
   
   📌 **Use this** to see exactly what happens at each moment in time.

---

## 🎯 How to Use This Documentation

### For Quick Understanding:
1. Read Section 1 (Executive Summary) in `TINYTHREADS_DETAILED_REPORT.md`
2. View the System Architecture diagram in `TINYTHREADS_DIAGRAMS.md`
3. Scan the Animated Walkthrough for key moments

### For Deep Learning:
1. **Day 1**: Read the entire `TINYTHREADS_DETAILED_REPORT.md`
2. **Day 2**: Study all diagrams in `TINYTHREADS_DIAGRAMS.md`
3. **Day 3**: Follow the step-by-step `TINYTHREADS_ANIMATED_WALKTHROUGH.md`
4. **Day 4**: Trace through the actual code with your new understanding

### For Exam Preparation:
- **Core Concepts**: Sections 2-4 of the main report
- **Visual Memory**: All diagrams, especially state machine and sequence diagrams
- **Trace Execution**: Animation frames 1-19

### For Assignment 4:
- Review Section 8.2 (Limitations and Future Enhancements)
- Study mutex_block structure (currently unimplemented)
- Understand spawnWithDeadline() requirements
- Learn about scheduler_RR(), scheduler_RM(), scheduler_EDF()

---

## 🔑 Key Concepts Explained

### 1. Cooperative Scheduling
- Threads must **voluntarily** call `yield()` to give up the CPU
- No preemption (unlike Linux/Windows)
- Simple but requires disciplined programming

### 2. Context Switching
- Uses `setjmp()`/`longjmp()` for portable context switching
- Saves all registers (PC, SP, x0-x30) to `jmp_buf`
- Enables resuming execution from exact save point

### 3. Queue Management
- **freeQ**: Available threads (unused)
- **readyQ**: Threads ready to run (FIFO)
- **doneQ**: Completed threads (not used in basic version)
- **current**: Currently executing thread

### 4. Thread Lifecycle
```
FREE → (spawn) → READY → (dispatch) → RUNNING → (yield) → READY
  ↑                                      ↓
  └────────────(function returns)────────┘
```

### 5. Critical Sections
- Protected by `DISABLE()`/`ENABLE()` (interrupt masking)
- Prevents race conditions on shared queues
- ARM instructions: `cpsid i` (disable) / `cpsie i` (enable)

---

## 📊 System Statistics

| Property | Value |
|----------|-------|
| **Max Threads** | 5 (NTHREADS) |
| **Stack Size per Thread** | 1 KB (1024 bytes) |
| **Context Switch Time** | ~0.1-0.3 μs |
| **Total Memory Usage** | ~6 KB (for 5 threads) |
| **Scheduling Algorithm** | Round-Robin (cooperative) |
| **Concurrency Type** | Time-sliced (not parallel) |

---

## 🧪 Assignment 3 Part 1 Behavior

### What the Code Does:
```
Main Thread (computePrimes):
├─ Displays "T1: Prime 2"  [0.5s] → yield()
├─ (Thread 0 runs)
├─ Displays "T1: Prime 3"  [0.5s] → yield()
├─ (Thread 0 runs)
├─ Displays "T1: Prime 5"  [0.5s] → yield()
└─ ...continues forever

Thread 0 (computePower):
├─ Displays "T0: 0^2=0"    [0.5s] → yield()
├─ (Main runs)
├─ Displays "T0: 1^2=1"    [0.5s] → yield()
├─ (Main runs)
├─ Displays "T0: 2^2=4"    [0.5s] → yield()
└─ ...continues forever
```

### Output Pattern:
```
T1: Prime 2
T0: 0^2=0
T1: Prime 3
T0: 1^2=1
T1: Prime 5
T0: 2^2=4
T1: Prime 7
T0: 3^2=9
...
```

Each thread gets **exactly 50%** of CPU time due to round-robin alternation.

---

## 🔬 How It Works (Simplified)

### spawn(computePower, 0):
1. Get free thread from `freeQ`
2. Set `thread->function = computePower`
3. Set `thread->arg = 0`
4. Save initial context with `setjmp()`
5. Setup stack pointer to thread's private stack
6. Add thread to `readyQ`

### yield():
1. **Disable interrupts** (enter critical section)
2. Check if other threads waiting in `readyQ`
3. If yes:
   - Dequeue next thread
   - Enqueue current thread to back of `readyQ`
   - **Context switch**: Save current, restore next
4. **Enable interrupts** (exit critical section)

### Context Switch (dispatch):
1. `setjmp(current->context)` → Save current thread's CPU state
2. `current = next` → Update current pointer
3. `longjmp(next->context, 1)` → Restore next thread's CPU state
4. Execution continues in next thread!

---

## 🎓 Learning Objectives Achieved

By studying this documentation, you will understand:

✅ How user-space threading works without OS support  
✅ The mechanics of context switching (setjmp/longjmp)  
✅ Queue-based thread scheduling algorithms  
✅ Critical section protection using interrupt disabling  
✅ The difference between cooperative and preemptive multitasking  
✅ Memory layout of thread control blocks  
✅ Stack management for multiple execution contexts  
✅ Why race conditions don't occur in cooperative systems  

---

## 🚀 Future Enhancements (Assignment 4)

The following features are **stubbed** but not implemented:

1. **Mutex Support** (`lock()`/`unlock()`)
   - Protect shared resources
   - Implement wait queues

2. **Priority Scheduling**
   - `spawnWithDeadline()` for periodic tasks
   - Rate Monotonic (RM) scheduling
   - Earliest Deadline First (EDF) scheduling

3. **Preemptive Scheduling**
   - Timer interrupt integration
   - Round-robin with time slicing
   - Automatic context switches

4. **Periodic Tasks**
   - `respawn_periodic_tasks()` implementation
   - Move threads from doneQ to readyQ
   - Period-based activation

---

## 📝 Quick Reference

### Important Functions

| Function | Purpose | Blocking? |
|----------|---------|-----------|
| `spawn(f, arg)` | Create new thread | No |
| `yield()` | Give up CPU | No (switches) |
| `lock(mutex)` | Acquire mutex | Yes (if locked) |
| `unlock(mutex)` | Release mutex | No |

### Important Data Structures

| Structure | Fields | Purpose |
|-----------|--------|---------|
| `thread_block` | idx, function, arg, context, stack | Thread control block |
| `mutex_block` | locked, waitQ | Synchronization (future) |
| `jmp_buf` | CPU registers | Saved context |

### Key Macros

| Macro | Purpose | ARM Instruction |
|-------|---------|-----------------|
| `DISABLE()` | Disable interrupts | `cpsid i` |
| `ENABLE()` | Enable interrupts | `cpsie i` |
| `SETSTACK(buf, stk)` | Setup stack pointer | Memory write |

---

## 🔗 Related Files in Assignment

- **Source Code**: `a3p1.c` (main application)
- **Library**: `lib/tinythreads.c` (threading implementation)
- **Header**: `lib/tinythreads.h` (API definitions)
- **Hardware**: `lib/piface.c` (LCD display)

---

## 📞 Additional Resources

- **Mermaid Diagrams**: [mermaid.live](https://mermaid.live) (paste diagrams to render)
- **ARM Documentation**: ARMv8 Architecture Reference Manual
- **setjmp/longjmp**: POSIX C Library documentation
- **Embedded Threading**: FreeRTOS documentation (similar concepts)

---

## ✨ Summary

TinyThreads is an elegant **proof-of-concept** showing that sophisticated multithreading can be achieved with:
- **~370 lines of C code**
- **No operating system**
- **Minimal memory overhead**
- **Simple, understandable design**

It sacrifices features (preemption, priorities) for simplicity and clarity, making it an **excellent teaching tool** for understanding threading fundamentals.

---

**Happy Learning! 🎉**

*For questions, refer back to the detailed report or trace through the code with the animated walkthrough.*

