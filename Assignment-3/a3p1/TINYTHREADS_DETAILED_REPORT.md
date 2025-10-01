# TinyThreads: Detailed Technical Report
## A Cooperative Multithreading System for ARMv8

**Authors:** Based on code by Johan Nordlander, Fredrik Bengtsson, Wagner de Morais, and Hazem Ali  
**Date:** October 1, 2025  
**Assignment:** DT8025 - Assignment 3, Part 1

---

## Table of Contents
1. [Executive Summary](#executive-summary)
2. [System Architecture](#system-architecture)
3. [Data Structures](#data-structures)
4. [Core Mechanisms](#core-mechanisms)
5. [Thread Lifecycle](#thread-lifecycle)
6. [Concurrency Management](#concurrency-management)
7. [Step-by-Step Execution Flow](#step-by-step-execution-flow)
8. [Implementation Analysis](#implementation-analysis)
9. [Conclusion](#conclusion)

---

## 1. Executive Summary

TinyThreads is a **cooperative multithreading library** designed for embedded systems running on ARMv8 architecture (Raspberry Pi 3). Unlike preemptive multitasking where the OS forcibly switches between threads, TinyThreads relies on threads voluntarily yielding control through the `yield()` function.

### Key Characteristics:
- **Cooperative Scheduling**: Threads must explicitly call `yield()` to allow other threads to run
- **Fixed Pool**: Maximum of 5 threads (configurable via `NTHREADS`)
- **Stack-based Execution**: Each thread has its own 1KB stack
- **Context Switching**: Uses `setjmp`/`longjmp` for saving/restoring execution context
- **Queue-based Management**: Three queues manage thread states (free, ready, done)
- **No Preemption**: Critical sections protected by interrupt disable/enable

---

## 2. System Architecture

### 2.1 High-Level Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    TinyThreads System                        │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐             │
│  │  freeQ   │───▶│ readyQ   │───▶│  doneQ   │             │
│  │(unused)  │    │(ready to │    │(finished)│             │
│  │          │    │  run)    │    │          │             │
│  └──────────┘    └──────────┘    └──────────┘             │
│       ▲               │               ▲                     │
│       │               │               │                     │
│       │          ┌────▼────┐          │                     │
│       │          │ current │          │                     │
│       │          │ thread  │          │                     │
│       │          └─────────┘          │                     │
│       │                               │                     │
│       └───────────(recycle)───────────┘                     │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐│
│  │         Thread Pool (5 thread_blocks)                  ││
│  │  [Thread 0][Thread 1][Thread 2][Thread 3][Thread 4]   ││
│  └────────────────────────────────────────────────────────┘│
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 Memory Organization

```
┌─────────────────────────────────────────────────────────────┐
│                     Memory Layout                            │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Global Variables:                                           │
│  ┌────────────────────────────────────────┐                │
│  │ thread_block threads[5]                │                │
│  │ ├─ threads[0]: 1KB + metadata          │                │
│  │ ├─ threads[1]: 1KB + metadata          │                │
│  │ ├─ threads[2]: 1KB + metadata          │                │
│  │ ├─ threads[3]: 1KB + metadata          │                │
│  │ └─ threads[4]: 1KB + metadata          │                │
│  └────────────────────────────────────────┘                │
│                                                              │
│  ┌────────────────────────────────────────┐                │
│  │ thread_block initp (main/init thread)  │                │
│  └────────────────────────────────────────┘                │
│                                                              │
│  Queue Pointers:                                             │
│  ┌────────────────────────────────────────┐                │
│  │ thread freeQ  ───▶ linked list         │                │
│  │ thread readyQ ───▶ linked list         │                │
│  │ thread doneQ  ───▶ NULL (future use)   │                │
│  │ thread current ──▶ currently executing │                │
│  └────────────────────────────────────────┘                │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 3. Data Structures

### 3.1 Thread Control Block

```c
struct thread_block {
    short idx;                      // Unique identifier (0-4)
    void (*function)(int);          // Function pointer to thread code
    int arg;                        // Argument passed to function
    thread next;                    // Pointer to next thread in queue
    jmp_buf context;                // Saved machine state (registers)
    char stack[STACKSIZE];          // 1024-byte execution stack
    unsigned int Period_Deadline;    // Absolute deadline (for future use)
    unsigned int Rel_Period_Deadline;// Relative deadline (for future use)
};
```

**Visual Representation:**

```
┌─────────────────────────────────────────────────────────────┐
│              Thread Control Block (thread_block)             │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────┐                                               │
│  │   idx    │  Short integer: 0, 1, 2, 3, or 4             │
│  └──────────┘                                               │
│  ┌──────────────────────────────────┐                      │
│  │  function pointer                │                       │
│  │  Points to: computePrimes,       │                       │
│  │            computePower, etc.    │                       │
│  └──────────────────────────────────┘                      │
│  ┌──────────┐                                               │
│  │   arg    │  Integer argument (e.g., segment number)     │
│  └──────────┘                                               │
│  ┌──────────┐                                               │
│  │   next   │─────────▶ Points to next thread in queue     │
│  └──────────┘                                               │
│  ┌──────────────────────────────────┐                      │
│  │   context (jmp_buf)              │                       │
│  │   Saved CPU state:               │                       │
│  │   - Program Counter (PC)         │                       │
│  │   - Stack Pointer (SP)           │                       │
│  │   - Registers (x0-x30)           │                       │
│  └──────────────────────────────────┘                      │
│  ┌──────────────────────────────────┐                      │
│  │   stack[1024]                    │                       │
│  │   Private execution stack        │                       │
│  │   Growing downward ▼             │                       │
│  └──────────────────────────────────┘                      │
│  ┌──────────────────────────────────┐                      │
│  │   Period_Deadline                │                       │
│  │   Rel_Period_Deadline            │                       │
│  │   (Reserved for future scheduling)│                      │
│  └──────────────────────────────────┘                      │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 3.2 Queue Management

The system uses three linked-list queues:

```
┌─────────────────────────────────────────────────────────────┐
│                        Queue Structure                       │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  1. FREE QUEUE (freeQ)                                      │
│     ┌────────┐    ┌────────┐    ┌────────┐                │
│     │Thread 2│───▶│Thread 3│───▶│Thread 4│───▶ NULL       │
│     └────────┘    └────────┘    └────────┘                │
│     Available threads for spawning                          │
│                                                              │
│  2. READY QUEUE (readyQ)                                    │
│     ┌────────┐    ┌────────┐                               │
│     │Thread 0│───▶│Thread 1│───▶ NULL                      │
│     └────────┘    └────────┘                               │
│     Threads waiting to execute                              │
│                                                              │
│  3. DONE QUEUE (doneQ)                                      │
│     NULL                                                     │
│     Threads that have completed (not used in basic impl)    │
│                                                              │
│  4. CURRENT POINTER                                         │
│     ───▶ [Thread executing now]                             │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

**Queue Operations:**

```
┌─────────────────────────────────────────────────────────────┐
│                    Queue Operations                          │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ENQUEUE (Add to tail):                                     │
│                                                              │
│  Before: HEAD ──▶ [A] ──▶ [B] ──▶ NULL                     │
│                                                              │
│  After:  HEAD ──▶ [A] ──▶ [B] ──▶ [C] ──▶ NULL            │
│                                      ▲                       │
│                                    (new)                     │
│                                                              │
│  DEQUEUE (Remove from head):                                │
│                                                              │
│  Before: HEAD ──▶ [A] ──▶ [B] ──▶ [C] ──▶ NULL            │
│           return ▲                                           │
│                                                              │
│  After:  HEAD ──▶ [B] ──▶ [C] ──▶ NULL                     │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 4. Core Mechanisms

### 4.1 Context Switching (setjmp/longjmp)

Context switching is the heart of TinyThreads. It allows saving the current execution state and jumping to another thread.

```
┌─────────────────────────────────────────────────────────────┐
│                  Context Switch Mechanism                    │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  THREAD A (running)         THREAD B (suspended)            │
│  ┌──────────────┐          ┌──────────────┐                │
│  │   Registers  │          │   Registers  │                │
│  │   x0 = ...   │          │   x0 = ...   │◀───┐           │
│  │   x1 = ...   │          │   x1 = ...   │    │           │
│  │   PC = ...   │          │   PC = ...   │    │           │
│  │   SP = ...   │          │   SP = ...   │    │           │
│  └──────────────┘          └──────────────┘    │           │
│         │                          ▲            │           │
│         │                          │            │           │
│         ▼ setjmp(A->context)       │ longjmp(B->context)   │
│  ┌──────────────┐          ┌──────────────┐    │           │
│  │ A->context   │          │ B->context   │────┘           │
│  │ (saved)      │          │ (restored)   │                │
│  └──────────────┘          └──────────────┘                │
│                                                              │
│  Step-by-step:                                              │
│  1. setjmp(A->context) saves Thread A's CPU state           │
│  2. Returns 0 on first call (save operation)                │
│  3. current = B (update current thread pointer)             │
│  4. longjmp(B->context, 1) restores Thread B's state        │
│  5. Execution continues in Thread B                         │
│  6. setjmp returns 1 (resumed from longjmp)                 │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 Thread Spawning Process

```
┌─────────────────────────────────────────────────────────────┐
│                    spawn(function, arg)                      │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  STEP 1: Allocate Thread                                    │
│  ┌────────────────────────────────┐                        │
│  │ DISABLE interrupts             │                         │
│  │ newp = dequeue(&freeQ)         │                         │
│  └────────────────────────────────┘                        │
│           │                                                  │
│           ▼                                                  │
│  ┌────────────────────────────────┐                        │
│  │ Initialize thread:             │                         │
│  │ - newp->function = function    │                         │
│  │ - newp->arg = arg              │                         │
│  │ - newp->next = NULL            │                         │
│  └────────────────────────────────┘                        │
│           │                                                  │
│           ▼                                                  │
│  STEP 2: Setup Context                                      │
│  ┌────────────────────────────────┐                        │
│  │ if (setjmp(newp->context) == 1)│ ◀─┐                    │
│  │    // Thread starts here       │   │                    │
│  │    ENABLE interrupts            │   │                    │
│  │    function(arg)                │   │ longjmp           │
│  │    // When function returns:    │   │ (later)           │
│  │    enqueue(current, &freeQ)     │   │                    │
│  │    dispatch(dequeue(&readyQ))   │   │                    │
│  └────────────────────────────────┘   │                    │
│           │                             │                    │
│           ▼                             │                    │
│  STEP 3: Setup Stack                    │                    │
│  ┌────────────────────────────────┐    │                    │
│  │ SETSTACK(&newp->context,       │    │                    │
│  │          &newp->stack)          │    │                    │
│  │ // Sets SP to top of stack     │    │                    │
│  └────────────────────────────────┘    │                    │
│           │                             │                    │
│           ▼                             │                    │
│  STEP 4: Add to Ready Queue             │                    │
│  ┌────────────────────────────────┐    │                    │
│  │ enqueue(newp, &readyQ)         │    │                    │
│  │ ENABLE interrupts              │    │                    │
│  └────────────────────────────────┘    │                    │
│           │                             │                    │
│           └─────────────────────────────┘                    │
│              (Thread will start when dispatched)             │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 4.3 Yielding (Cooperative Scheduling)

```
┌─────────────────────────────────────────────────────────────┐
│                      yield() Function                        │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Current thread voluntarily gives up CPU                     │
│                                                              │
│  BEFORE yield():                                             │
│  ┌──────────────────────────────────────┐                  │
│  │ current ──▶ [Thread A] (running)     │                  │
│  │                                       │                  │
│  │ readyQ  ──▶ [Thread B] ─▶ [Thread C] │                  │
│  └──────────────────────────────────────┘                  │
│                                                              │
│  STEP 1: Disable Interrupts                                 │
│  ┌──────────────────────────────────────┐                  │
│  │ DISABLE()  // Critical section start  │                  │
│  └──────────────────────────────────────┘                  │
│                                                              │
│  STEP 2: Check Ready Queue                                  │
│  ┌──────────────────────────────────────┐                  │
│  │ if (readyQ != NULL) {                 │                  │
│  │   // There are other threads waiting  │                  │
│  │ }                                     │                  │
│  └──────────────────────────────────────┘                  │
│                                                              │
│  STEP 3: Dequeue Next Thread                                │
│  ┌──────────────────────────────────────┐                  │
│  │ p = dequeue(&readyQ)                  │                  │
│  │ // p now points to Thread B           │                  │
│  └──────────────────────────────────────┘                  │
│                                                              │
│  STEP 4: Move Current to Back of Queue                      │
│  ┌──────────────────────────────────────┐                  │
│  │ enqueue(current, &readyQ)             │                  │
│  │ // Thread A goes to back of queue     │                  │
│  └──────────────────────────────────────┘                  │
│                                                              │
│  STEP 5: Context Switch                                     │
│  ┌──────────────────────────────────────┐                  │
│  │ dispatch(p)                            │                  │
│  │ // Save A's context, restore B's      │                  │
│  └──────────────────────────────────────┘                  │
│                                                              │
│  STEP 6: Enable Interrupts (after switch)                   │
│  ┌──────────────────────────────────────┐                  │
│  │ ENABLE()  // Critical section end     │                  │
│  └──────────────────────────────────────┘                  │
│                                                              │
│  AFTER yield():                                              │
│  ┌──────────────────────────────────────┐                  │
│  │ current ──▶ [Thread B] (running)     │                  │
│  │                                       │                  │
│  │ readyQ  ──▶ [Thread C] ─▶ [Thread A] │                  │
│  └──────────────────────────────────────┘                  │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 5. Thread Lifecycle

### 5.1 State Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                   Thread State Transitions                   │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│                    ┌────────────┐                           │
│                    │    FREE    │                           │
│                    │  (unused)  │                           │
│                    └────────────┘                           │
│                         │   ▲                                │
│                  spawn()│   │function                        │
│                         │   │returns                         │
│                         ▼   │                                │
│                    ┌────────────┐                           │
│              ┌────▶│   READY    │◀────┐                     │
│              │     │ (runnable) │     │                     │
│              │     └────────────┘     │                     │
│              │          │   ▲         │                     │
│         yield()    dispatch│   │yield()                     │
│              │          │   │         │                     │
│              │          ▼   │         │                     │
│              │     ┌────────────┐     │                     │
│              └─────│  RUNNING   │─────┘                     │
│                    │ (current)  │                           │
│                    └────────────┘                           │
│                                                              │
│  State Descriptions:                                         │
│  ┌────────────────────────────────────────────────────┐    │
│  │ FREE:    Thread available for allocation           │    │
│  │          Lives in freeQ                             │    │
│  ├────────────────────────────────────────────────────┤    │
│  │ READY:   Thread ready to execute                   │    │
│  │          Lives in readyQ                            │    │
│  │          Waiting for CPU time                       │    │
│  ├────────────────────────────────────────────────────┤    │
│  │ RUNNING: Thread currently executing                │    │
│  │          Pointed to by 'current'                    │    │
│  │          Has control of CPU                         │    │
│  └────────────────────────────────────────────────────┘    │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 5.2 Complete Lifecycle Example

```
┌─────────────────────────────────────────────────────────────┐
│          Thread Lifecycle: From Birth to Death              │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  TIME 0: System Initialization                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ initializeThreads()                                   │  │
│  │ All 5 threads → freeQ                                 │  │
│  │ freeQ: [0]→[1]→[2]→[3]→[4]→NULL                     │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                              │
│  TIME 1: First spawn(computePower, 0)                       │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Thread 0 removed from freeQ                           │  │
│  │ Thread 0 configured for computePower                  │  │
│  │ Thread 0 added to readyQ                              │  │
│  │                                                        │  │
│  │ freeQ:  [1]→[2]→[3]→[4]→NULL                         │  │
│  │ readyQ: [0]→NULL                                      │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                              │
│  TIME 2: Main calls computePrimes(1) [no spawn]             │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Main thread enters infinite loop                      │  │
│  │ current: main (initp)                                 │  │
│  │ readyQ: [0]→NULL                                      │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                              │
│  TIME 3: First yield() in computePrimes                     │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Main thread yields                                    │  │
│  │ Thread 0 dispatched                                   │  │
│  │ Main thread added to readyQ                           │  │
│  │                                                        │  │
│  │ current: Thread 0 (computePower)                      │  │
│  │ readyQ: [main]→NULL                                   │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                              │
│  TIME 4: Thread 0 yields                                    │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Thread 0 yields after computing 0^2=0                 │  │
│  │ Main thread dispatched                                │  │
│  │ Thread 0 added to back of readyQ                      │  │
│  │                                                        │  │
│  │ current: Main (computePrimes)                         │  │
│  │ readyQ: [0]→NULL                                      │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                              │
│  TIME 5+: Steady State (Round-Robin)                        │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Threads alternate forever:                            │  │
│  │ Main → T0 → Main → T0 → Main → T0 → ...             │  │
│  │                                                        │  │
│  │ Each thread:                                          │  │
│  │ 1. Computes next value                                │  │
│  │ 2. Displays to PiFace                                 │  │
│  │ 3. Waits 0.5 seconds                                  │  │
│  │ 4. Calls yield()                                      │  │
│  │ 5. Returns to readyQ                                  │  │
│  │ 6. Waits for next turn                                │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 6. Concurrency Management

### 6.1 Critical Sections

TinyThreads uses **interrupt disabling** to protect critical sections:

```
┌─────────────────────────────────────────────────────────────┐
│              Critical Section Protection                     │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  WITHOUT Protection (DANGEROUS):                            │
│  ┌────────────────────────────────────────────────────┐    │
│  │ Thread A: p = dequeue(&readyQ)  // Gets Thread 1   │    │
│  │           ▼ INTERRUPT ▼                             │    │
│  │ Handler:  p = dequeue(&readyQ)  // Also gets T1!    │    │
│  │           ▲ RETURN ▲                                │    │
│  │ Thread A: enqueue(p, &freeQ)    // Corrupts queue!  │    │
│  └────────────────────────────────────────────────────┘    │
│                         ❌ DATA RACE                         │
│                                                              │
│  WITH Protection (SAFE):                                    │
│  ┌────────────────────────────────────────────────────┐    │
│  │ Thread A: DISABLE()              // Block interrupts│    │
│  │           p = dequeue(&readyQ)   // Gets Thread 1   │    │
│  │           enqueue(p, &freeQ)                         │    │
│  │           ENABLE()               // Allow interrupts│    │
│  └────────────────────────────────────────────────────┘    │
│                         ✓ ATOMIC OPERATION                   │
│                                                              │
│  ARM Assembly Implementation:                               │
│  ┌────────────────────────────────────────────────────┐    │
│  │ disable():  cpsid i    // Set interrupt mask       │    │
│  │ enable():   cpsie i    // Clear interrupt mask     │    │
│  └────────────────────────────────────────────────────┘    │
│                                                              │
│  Critical Sections in TinyThreads:                          │
│  ┌────────────────────────────────────────────────────┐    │
│  │ • spawn(): Entire function (queue manipulation)     │    │
│  │ • yield(): Entire function (context switch)         │    │
│  │ • lock()/unlock(): Mutex operations (future)        │    │
│  └────────────────────────────────────────────────────┘    │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 6.2 No Race Conditions (By Design)

```
┌─────────────────────────────────────────────────────────────┐
│           Why TinyThreads Avoids Race Conditions            │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  1. COOPERATIVE SCHEDULING                                  │
│     ┌──────────────────────────────────────────────────┐   │
│     │ Threads only switch at explicit yield() points   │   │
│     │ No preemption during computation                  │   │
│     │ Developer controls context switches               │   │
│     └──────────────────────────────────────────────────┘   │
│                                                              │
│  2. INTERRUPT PROTECTION                                    │
│     ┌──────────────────────────────────────────────────┐   │
│     │ All queue operations wrapped in DISABLE/ENABLE   │   │
│     │ Hardware interrupts cannot corrupt shared data    │   │
│     └──────────────────────────────────────────────────┘   │
│                                                              │
│  3. SINGLE EXECUTION FLOW                                   │
│     ┌──────────────────────────────────────────────────┐   │
│     │ Only ONE thread runs at any time                  │   │
│     │ 'current' pointer = sole active thread            │   │
│     │ No true parallelism (single-core execution)       │   │
│     └──────────────────────────────────────────────────┘   │
│                                                              │
│  4. PRIVATE STACKS                                          │
│     ┌──────────────────────────────────────────────────┐   │
│     │ Each thread has isolated 1KB stack               │   │
│     │ Local variables are thread-private                │   │
│     │ No stack corruption between threads               │   │
│     └──────────────────────────────────────────────────┘   │
│                                                              │
│  Shared Data That IS Protected:                             │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ • readyQ, freeQ, doneQ (linked lists)                │  │
│  │ • current (active thread pointer)                     │  │
│  │ • Thread metadata (idx, function, arg)               │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                              │
│  Data That COULD Race (if shared carelessly):               │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ • Global variables (e.g., counters)                   │  │
│  │ • Heap-allocated structures                           │  │
│  │ ⚠ Developer must use mutex (future Assignment 4)     │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 7. Step-by-Step Execution Flow

### 7.1 Program Startup Sequence

```
┌─────────────────────────────────────────────────────────────┐
│              Detailed Execution Timeline                     │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│ ╔═══════════════════════════════════════════════════════╗  │
│ ║ PHASE 1: System Initialization (in main)              ║  │
│ ╚═══════════════════════════════════════════════════════╝  │
│                                                              │
│  [1] piface_init()                                          │
│      └─ Initialize LCD display hardware                     │
│                                                              │
│  [2] piface_clear()                                         │
│      └─ Clear display                                        │
│                                                              │
│  [3] Display "DT8025 - A3P1"                                │
│      └─ Wait 2 seconds                                       │
│                                                              │
│  [4] piface_clear()                                         │
│      └─ Ready for thread output                             │
│                                                              │
│ ┌────────────────────────────────────────────────────────┐ │
│ │ State at this point:                                   │ │
│ │ freeQ:  [0]→[1]→[2]→[3]→[4]→NULL (uninitialized)     │ │
│ │ readyQ: NULL                                           │ │
│ │ current: &initp (main thread)                          │ │
│ └────────────────────────────────────────────────────────┘ │
│                                                              │
│ ╔═══════════════════════════════════════════════════════╗  │
│ ║ PHASE 2: Thread Creation                              ║  │
│ ╚═══════════════════════════════════════════════════════╝  │
│                                                              │
│  [5] spawn(computePower, 0) called                          │
│      │                                                       │
│      ├─ [5.1] DISABLE interrupts                            │
│      │                                                       │
│      ├─ [5.2] First spawn, so initializeThreads()           │
│      │        ┌───────────────────────────────────────┐    │
│      │        │ for each thread i=0 to 4:             │    │
│      │        │   threads[i].idx = i                   │    │
│      │        │   threads[i].function = NULL           │    │
│      │        │   threads[i].arg = -1                  │    │
│      │        │   threads[i].next = &threads[i+1]      │    │
│      │        │   threads[i].Period_Deadline = INT_MAX │    │
│      │        │ threads[4].next = NULL                 │    │
│      │        │ freeQ = threads (points to threads[0]) │    │
│      │        │ initialized = 1                        │    │
│      │        └───────────────────────────────────────┘    │
│      │                                                       │
│      │   State after initialization:                        │
│      │   freeQ: [0]→[1]→[2]→[3]→[4]→NULL                  │
│      │                                                       │
│      ├─ [5.3] newp = dequeue(&freeQ)                        │
│      │        newp now points to threads[0]                 │
│      │        freeQ: [1]→[2]→[3]→[4]→NULL                 │
│      │                                                       │
│      ├─ [5.4] Configure thread:                             │
│      │        newp->function = computePower                 │
│      │        newp->arg = 0                                 │
│      │        newp->next = NULL                             │
│      │                                                       │
│      ├─ [5.5] setjmp(newp->context) == 0 (save state)       │
│      │        // Will return 1 when thread starts later     │
│      │                                                       │
│      ├─ [5.6] SETSTACK(&newp->context, &newp->stack)        │
│      │        Sets stack pointer to top of thread's stack   │
│      │        SP = &(threads[0].stack[1023])                │
│      │                                                       │
│      ├─ [5.7] enqueue(newp, &readyQ)                        │
│      │        readyQ: [0]→NULL                              │
│      │                                                       │
│      └─ [5.8] ENABLE interrupts                             │
│                                                              │
│ ┌────────────────────────────────────────────────────────┐ │
│ │ State after spawn:                                     │ │
│ │ freeQ:  [1]→[2]→[3]→[4]→NULL                         │ │
│ │ readyQ: [0]→NULL                                       │ │
│ │ current: &initp (still main)                           │ │
│ └────────────────────────────────────────────────────────┘ │
│                                                              │
│ ╔═══════════════════════════════════════════════════════╗  │
│ ║ PHASE 3: Main Thread Execution                        ║  │
│ ╚═══════════════════════════════════════════════════════╝  │
│                                                              │
│  [6] computePrimes(1) called (not spawned!)                 │
│      │                                                       │
│      ├─ [6.1] n = 0                                          │
│      │        is_prime(0) = 0 (false)                       │
│      │                                                       │
│      ├─ [6.2] n = 1                                          │
│      │        is_prime(1) = 0 (false)                       │
│      │                                                       │
│      ├─ [6.3] n = 2                                          │
│      │        is_prime(2) = 1 (true)                        │
│      │        PUTTOLDC("T1: Prime 2")                       │
│      │        Display to PiFace LCD                         │
│      │        RPI_WaitMicroSeconds(500000) // 0.5s          │
│      │        yield() ◀─── FIRST CONTEXT SWITCH             │
│      │                                                       │
│      └─ [continues in Phase 4]                              │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 7.2 First Context Switch (Critical Moment)

```
┌─────────────────────────────────────────────────────────────┐
│          First yield() - The Magic Happens Here             │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Called from: computePrimes in main thread                  │
│  Purpose: Allow Thread 0 (computePower) to run              │
│                                                              │
│  ┌────────────────────────────────────────────────────┐    │
│  │ STEP 1: Enter yield() function                      │    │
│  │         current = &initp (main thread)              │    │
│  │         readyQ = [Thread 0]→NULL                    │    │
│  └────────────────────────────────────────────────────┘    │
│           │                                                  │
│           ▼                                                  │
│  ┌────────────────────────────────────────────────────┐    │
│  │ STEP 2: DISABLE()                                   │    │
│  │         CPU executes: cpsid i                       │    │
│  │         PRIMASK register = 1 (interrupts blocked)   │    │
│  └────────────────────────────────────────────────────┘    │
│           │                                                  │
│           ▼                                                  │
│  ┌────────────────────────────────────────────────────┐    │
│  │ STEP 3: Check readyQ != NULL                        │    │
│  │         TRUE (Thread 0 is waiting)                  │    │
│  └────────────────────────────────────────────────────┘    │
│           │                                                  │
│           ▼                                                  │
│  ┌────────────────────────────────────────────────────┐    │
│  │ STEP 4: p = dequeue(&readyQ)                        │    │
│  │         p = Thread 0 (computePower)                 │    │
│  │         readyQ = NULL                                │    │
│  └────────────────────────────────────────────────────┘    │
│           │                                                  │
│           ▼                                                  │
│  ┌────────────────────────────────────────────────────┐    │
│  │ STEP 5: enqueue(current, &readyQ)                   │    │
│  │         current = &initp (main)                     │    │
│  │         readyQ = [main]→NULL                        │    │
│  └────────────────────────────────────────────────────┘    │
│           │                                                  │
│           ▼                                                  │
│  ┌────────────────────────────────────────────────────┐    │
│  │ STEP 6: dispatch(p)                                 │    │
│  │         ┌───────────────────────────────────────┐  │    │
│  │         │ if (setjmp(current->context) == 0) {   │  │    │
│  │         │   // Save main's state                 │  │    │
│  │         │   // Saves: PC, SP, x0-x30 registers   │  │    │
│  │         │   // Returns 0 (first time)            │  │    │
│  │         │   current = next (Thread 0)            │  │    │
│  │         │   longjmp(next->context, 1)            │  │    │
│  │         │   // Restore Thread 0's state          │  │    │
│  │         │   // Jump to Thread 0's PC             │  │    │
│  │         │ }                                       │  │    │
│  │         └───────────────────────────────────────┘  │    │
│  └────────────────────────────────────────────────────┘    │
│           │                                                  │
│           │ ⚡ CONTEXT SWITCH HAPPENS ⚡                     │
│           │                                                  │
│           ▼                                                  │
│  ┌────────────────────────────────────────────────────┐    │
│  │ NOW IN THREAD 0                                     │    │
│  │ setjmp(newp->context) returns 1                     │    │
│  │ (from spawn, line 171)                              │    │
│  └────────────────────────────────────────────────────┘    │
│           │                                                  │
│           ▼                                                  │
│  ┌────────────────────────────────────────────────────┐    │
│  │ STEP 7: ENABLE()                                    │    │
│  │         (in spawn's if block, line 172)             │    │
│  └────────────────────────────────────────────────────┘    │
│           │                                                  │
│           ▼                                                  │
│  ┌────────────────────────────────────────────────────┐    │
│  │ STEP 8: current->function(current->arg)             │    │
│  │         computePower(0) starts executing!           │    │
│  └────────────────────────────────────────────────────┘    │
│                                                              │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ Memory State After Switch:                          │   │
│  │                                                      │   │
│  │ CPU Registers:                                       │   │
│  │   PC (Program Counter) ───▶ Inside computePower()   │   │
│  │   SP (Stack Pointer)   ───▶ threads[0].stack[1023]  │   │
│  │   x0, x1, ... x30      ───▶ Thread 0's saved values │   │
│  │                                                      │   │
│  │ TinyThreads State:                                   │   │
│  │   current ──▶ threads[0]                             │   │
│  │   readyQ  ──▶ [main]→NULL                           │   │
│  │   freeQ   ──▶ [1]→[2]→[3]→[4]→NULL                 │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 7.3 Steady-State Round-Robin

```
┌─────────────────────────────────────────────────────────────┐
│              Round-Robin Thread Alternation                  │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Timeline showing repeated yield cycles:                     │
│                                                              │
│  ┌──────────┐        ┌──────────┐        ┌──────────┐      │
│  │  Main    │ yield()│ Thread 0 │ yield()│   Main   │      │
│  │ (Prime 2)│───────▶│  (0^2=0) │───────▶│ (Prime 3)│      │
│  └──────────┘        └──────────┘        └──────────┘      │
│      │                                         │             │
│      │ Display "T1: Prime 2"                   │             │
│      │ Wait 0.5s                               │             │
│      │ yield() ───────────────┐                │             │
│      │                         ▼                │             │
│      │                  Switch to T0            │             │
│      │                         │                │             │
│      │                         │ Display "T0: 0^2=0"         │
│      │                         │ Wait 0.5s                    │
│      │                         │ yield() ──────┐             │
│      │                         ▼               │             │
│      │                  Switch to Main         │             │
│      │                                         │             │
│      │                  [Cycle repeats]        │             │
│      ▼                                         ▼             │
│                                                              │
│  Detailed Cycle (one complete rotation):                     │
│                                                              │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ ITERATION N:                                         │   │
│  │                                                      │   │
│  │ [A] Main Thread Running                              │   │
│  │     current: main                                    │   │
│  │     readyQ:  [T0]→NULL                               │   │
│  │     ─────────────────────────────                    │   │
│  │     1. Compute: is_prime(n)                          │   │
│  │     2. If prime: Display "T1: Prime n"               │   │
│  │     3. Wait 500ms                                    │   │
│  │     4. yield()                                       │   │
│  │        ├─ Save main's context                        │   │
│  │        ├─ Dequeue T0                                 │   │
│  │        ├─ Enqueue main                               │   │
│  │        └─ Restore T0's context                       │   │
│  │                                                      │   │
│  │ [B] Thread 0 Running                                 │   │
│  │     current: T0                                      │   │
│  │     readyQ:  [main]→NULL                             │   │
│  │     ─────────────────────────────                    │   │
│  │     1. Compute: n^2                                  │   │
│  │     2. Display "T0: n^2=result"                      │   │
│  │     3. Wait 500ms                                    │   │
│  │     4. yield()                                       │   │
│  │        ├─ Save T0's context                          │   │
│  │        ├─ Dequeue main                               │   │
│  │        ├─ Enqueue T0                                 │   │
│  │        └─ Restore main's context                     │   │
│  │                                                      │   │
│  │ [CYCLE REPEATS FROM A]                               │   │
│  │                                                      │   │
│  │ Each thread gets equal CPU time!                     │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                              │
│  Example Output Sequence:                                   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ T1: Prime 2     (Main, 0.5s)                         │   │
│  │ T0: 0^2=0       (Thread 0, 0.5s)                     │   │
│  │ T1: Prime 3     (Main, 0.5s)                         │   │
│  │ T0: 1^2=1       (Thread 0, 0.5s)                     │   │
│  │ T1: Prime 5     (Main, 0.5s)                         │   │
│  │ T0: 2^2=4       (Thread 0, 0.5s)                     │   │
│  │ T1: Prime 7     (Main, 0.5s)                         │   │
│  │ T0: 3^2=9       (Thread 0, 0.5s)                     │   │
│  │ ...                                                  │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 8. Implementation Analysis

### 8.1 Performance Characteristics

```
┌─────────────────────────────────────────────────────────────┐
│                Performance Analysis                          │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Context Switch Cost:                                        │
│  ┌────────────────────────────────────────────────────┐    │
│  │ setjmp():  ~50-100 CPU cycles                       │    │
│  │   - Save ~30 registers (x0-x30, PC, SP, etc.)      │    │
│  │   - Memory writes to jmp_buf                        │    │
│  │                                                     │    │
│  │ longjmp(): ~50-100 CPU cycles                       │    │
│  │   - Restore ~30 registers                           │    │
│  │   - Jump to new PC                                  │    │
│  │                                                     │    │
│  │ Queue operations: ~10-20 cycles each                │    │
│  │                                                     │    │
│  │ Total per context switch: ~120-250 cycles          │    │
│  │                        ≈ 0.1-0.3 microseconds      │    │
│  │                          (on 1GHz CPU)              │    │
│  └────────────────────────────────────────────────────┘    │
│                                                              │
│  Memory Usage:                                              │
│  ┌────────────────────────────────────────────────────┐    │
│  │ Per Thread:                                         │    │
│  │   Stack:           1024 bytes                       │    │
│  │   Context:         ~200 bytes (jmp_buf)            │    │
│  │   Metadata:        ~24 bytes                        │    │
│  │   Total:           ~1248 bytes                      │    │
│  │                                                     │    │
│  │ For 5 Threads:     ~6240 bytes (6 KB)              │    │
│  │                                                     │    │
│  │ Very lightweight for embedded systems!              │    │
│  └────────────────────────────────────────────────────┘    │
│                                                              │
│  Advantages:                                                │
│  ┌────────────────────────────────────────────────────┐    │
│  │ ✓ Simple to understand and debug                   │    │
│  │ ✓ Deterministic (no unexpected preemption)          │    │
│  │ ✓ Low memory overhead                               │    │
│  │ ✓ Fast context switches                             │    │
│  │ ✓ No race conditions on shared data (by design)    │    │
│  │ ✓ Easy to reason about program flow                │    │
│  └────────────────────────────────────────────────────┘    │
│                                                              │
│  Disadvantages:                                             │
│  ┌────────────────────────────────────────────────────┐    │
│  │ ✗ Threads must explicitly yield (cooperative)      │    │
│  │ ✗ One misbehaving thread can block all others      │    │
│  │ ✗ No prioritization (simple round-robin)           │    │
│  │ ✗ Fixed number of threads (NTHREADS=5)             │    │
│  │ ✗ No built-in synchronization primitives (yet)     │    │
│  └────────────────────────────────────────────────────┘    │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 8.2 Limitations and Future Enhancements

```
┌─────────────────────────────────────────────────────────────┐
│            Current Limitations & Assignment 4 TODOs         │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Unimplemented Features:                                    │
│                                                              │
│  1. Mutex Support (lock/unlock)                             │
│     ┌──────────────────────────────────────────────────┐   │
│     │ Currently stubs only                              │   │
│     │ Needed for protecting shared resources            │   │
│     │ Will use waitQ in mutex_block                     │   │
│     └──────────────────────────────────────────────────┘   │
│                                                              │
│  2. Priority Scheduling                                     │
│     ┌──────────────────────────────────────────────────┐   │
│     │ spawnWithDeadline() not implemented               │   │
│     │ Need insertion sort in enqueue()                  │   │
│     │ Enable Rate Monotonic (RM) scheduling             │   │
│     └──────────────────────────────────────────────────┘   │
│                                                              │
│  3. Earliest Deadline First (EDF)                           │
│     ┌──────────────────────────────────────────────────┐   │
│     │ scheduler_EDF() stub                              │   │
│     │ Requires dynamic deadline tracking                │   │
│     │ Sort readyQ by absolute deadline                  │   │
│     └──────────────────────────────────────────────────┘   │
│                                                              │
│  4. Round-Robin with Time Slicing                           │
│     ┌──────────────────────────────────────────────────┐   │
│     │ scheduler_RR() stub                               │   │
│     │ Needs timer interrupt integration                 │   │
│     │ Preemptive context switch on timer tick           │   │
│     └──────────────────────────────────────────────────┘   │
│                                                              │
│  5. Periodic Task Re-spawning                               │
│     ┌──────────────────────────────────────────────────┐   │
│     │ respawn_periodic_tasks() stub                     │   │
│     │ Move threads from doneQ back to readyQ            │   │
│     │ Check if period elapsed                           │   │
│     └──────────────────────────────────────────────────┘   │
│                                                              │
│  Potential Enhancements:                                    │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ • Dynamic thread allocation (vs fixed pool)          │   │
│  │ • Thread sleep/wakeup mechanisms                     │   │
│  │ • Semaphores and condition variables                │   │
│  │ • Inter-thread messaging                             │   │
│  │ • Thread priorities with aging                       │   │
│  │ • Stack overflow detection                           │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 9. Conclusion

### Key Takeaways

1. **Simplicity**: TinyThreads demonstrates that multithreading doesn't require a full OS. With just ~370 lines of C code, we have a functional cooperative multitasking system.

2. **Cooperative Model**: The explicit `yield()` requirement makes program flow predictable and easier to debug, though it requires discipline from programmers.

3. **Hardware Integration**: Uses ARM-specific instructions (`cpsid i`, `cpsie i`) for interrupt control, showing low-level hardware awareness.

4. **Queue-based Design**: All thread management relies on three simple linked lists (freeQ, readyQ, doneQ), making the system transparent and traceable.

5. **Context Switching Magic**: The `setjmp`/`longjmp` mechanism provides portable context switching without writing assembly for every platform.

### Real-World Applications

This design pattern is used in:
- **Embedded Systems**: Arduino, FreeRTOS (cooperative mode)
- **Game Engines**: Coroutines for game logic
- **Event-Driven Systems**: Node.js (event loop), Python asyncio
- **Real-Time Systems**: When deterministic behavior is critical

### Learning Outcomes

After studying TinyThreads, you understand:
- How threads are represented in memory
- The mechanics of context switching
- Queue-based scheduling algorithms
- Critical section protection
- The trade-offs between cooperative and preemptive multitasking

---

**End of Report**

*For questions or clarifications, refer to the source code in `lib/tinythreads.c` or consult the assignment documentation.*

