# Main Thread Scheduling Issue: Why computeSomethingForever Fails in Main

## Overview

This document explains why the TinyThreads program works with 4 spawned threads and a no-operation loop in main, but fails when using 3 spawned threads with `computeSomethingForever` running directly in the main thread.

## The Problem

### Working Configuration
```c
spawn(computeSomethingForever, 0);
spawn(computeSomethingForever, 1);
spawn(computeSomethingForever, 2);
spawn(computeSomethingForever, 3);

// Main thread becomes idle loop
while(1) {
    no_operation();
}
```
✅ **Result:** Works correctly - all 4 threads execute and context switch properly.

### Failing Configuration
```c
spawn(computeSomethingForever, 0);
spawn(computeSomethingForever, 1);
spawn(computeSomethingForever, 2);
computeSomethingForever(3);  // Running in main's context
```
❌ **Result:** Crashes, hangs, or exhibits undefined behavior.

## Root Cause Analysis

### 1. Thread Initialization Differences

#### Spawned Threads (Properly Initialized)
When a thread is spawned via `spawn()` in `tinythreads.c` (lines 152-172):
1. **Dedicated Stack Allocation:** `SETSTACK(&newp->context, &newp->stack)` (line 169)
   - Each thread gets its own 1024-byte stack space
   - Stack pointer is properly set in the thread's context
2. **Context Saving:** `setjmp(newp->context)` (line 161)
   - Saves all CPU registers and stack pointer
   - Creates a valid restoration point for context switching
3. **Ready Queue:** Thread is enqueued to `readyQ` with complete initialization

#### Main Thread `initp` (Minimally Initialized)
Looking at `tinythreads.c` lines 78-84:
```c
initp.idx = -1;
initp.function = NULL;
initp.arg = -1;
initp.next = NULL;
initp.Period_Deadline = INT_MAX;
initp.Rel_Period_Deadline = INT_MAX;
```
**Critical Missing Steps:**
- ❌ No `SETSTACK()` call - uses system's main stack
- ❌ No dedicated thread stack allocation
- ❌ Context is saved on-the-fly but with incompatible stack

### 2. The Stack Problem

#### Stack Layout Comparison

**Spawned Thread:**
```
┌─────────────────────────┐
│  Thread Stack (1024B)   │ ← Dedicated, isolated
│  STACKSIZE allocated    │
│  Via SETSTACK macro     │
└─────────────────────────┘
```

**Main Thread (initp):**
```
┌─────────────────────────┐
│  System Main Stack      │ ← Shared with startup code
│  Size unknown/variable  │
│  NOT thread-allocated   │
└─────────────────────────┘
```

### 3. What Happens When Main Calls yield()

When `computeSomethingForever(3)` runs in main and calls `yield()`:

```c
void yield(void) {
    DISABLE();
    if (readyQ != NULL){		
        thread p = dequeue(&readyQ);
        enqueue(current, &readyQ);  // ← Puts 'initp' in readyQ!
        dispatch(p);
    }	
    ENABLE();
}
```

**The Problem:**
1. `current` points to `initp` (the main thread)
2. `initp` gets enqueued to `readyQ` alongside properly initialized threads
3. `dispatch()` saves `initp`'s context via `setjmp(current->context)`
4. **But `initp` has no dedicated stack!** The stack pointer saved references the main system stack

### 4. Timer Interrupt Catastrophe

Every 0.25 seconds, the ARM timer fires an interrupt (`rpi-interrupts.c` line 95-106):

```c
void __attribute__((interrupt("IRQ"))) interrupt_vector(void)
{
    if( RPI_GetArmTimer()->MaskedIRQ ) {
        RPI_GetArmTimer()->IRQClear = 1;
        ticks++;
        print2uart("ticks: %d\n", ticks);
        scheduler();  // ← Context switch during interrupt!
    }
}
```

**Execution Flow:**
```
computeSomethingForever(3) running in main
    ↓
Timer Interrupt Fires
    ↓
interrupt_vector() called
    ↓
scheduler() → scheduler_RR()
    ↓
dispatch() tries to context switch
    ↓
setjmp(current->context)  ← Saving 'initp' context
    ↓
longjmp(next->context)    ← Jump to another thread
    ↓
[Later] longjmp back to initp
    ↓
💥 STACK CORRUPTION / INVALID STACK POINTER
```

### 5. Why Stack Corruption Occurs

**Interrupt Stack Frames:**
- ARM processors use different stack modes for interrupts (IRQ mode)
- When an interrupt fires during `computeSomethingForever` in main:
  1. CPU switches to IRQ stack
  2. Saves return address and registers
  3. Calls `interrupt_vector()`
  4. `scheduler()` attempts context switch
  5. `setjmp/longjmp` expects thread-isolated stacks
  6. **Conflict:** Main's stack is not isolated; interrupt return address might get overwritten

**Memory Layout Conflict:**
```
Main Stack (used by initp):
┌──────────────────────────┐
│ computeSomethingForever  │
│ local variables          │
├──────────────────────────┤ ← Interrupt occurs here
│ IRQ stack frame pushed   │
│ (return addr, registers) │
├──────────────────────────┤
│ scheduler() stack frame  │
│ dispatch() stack frame   │
├──────────────────────────┤
│ setjmp saves THIS state  │ ← Points to corrupted stack!
└──────────────────────────┘
```

When `longjmp` restores this context later:
- Stack pointer might point to invalid/overwritten data
- Return addresses corrupted
- Undefined behavior: crashes, infinite loops, or silent failures

## Why No-Operation Loop Works

```c
while(1) {
    no_operation();
}
```

**Safe Behavior:**
1. Main thread (`initp`) **never calls `yield()`**
2. `initp` **never enters `readyQ`**
3. `scheduler_RR()` only switches among the 4 properly initialized spawned threads
4. All context switches happen between threads with dedicated, isolated stacks
5. Main thread continues spinning in the no-op loop, unaffected by context switches

**Thread Participation:**
- Only spawned threads (0, 1, 2, 3) participate in scheduling
- All have proper stack allocation via `SETSTACK`
- Context switches are safe and isolated

## Comparative Summary

| Aspect | 4 Threads + No-Op | 3 Threads + Main ComputeSomething |
|--------|-------------------|-----------------------------------|
| **Main thread (`initp`)** | Stays in simple loop, doesn't yield | Calls `yield()`, enters `readyQ` |
| **Stack setup** | All active threads have `SETSTACK` | `initp` has **no** dedicated stack |
| **Interrupt handling** | Safe - switches among spawned threads | **Unsafe** - tries to switch to/from `initp` |
| **Thread count in `readyQ`** | 4 properly initialized threads | 3 proper + 1 improperly initialized (`initp`) |
| **Stack isolation** | ✅ All threads have isolated stacks | ❌ `initp` shares system stack |
| **Context switch safety** | ✅ All contexts are valid | ❌ `initp` context has invalid stack pointer |
| **Result** | ✅ Works correctly | ❌ Crashes/hangs/undefined behavior |

## Design Philosophy: Main as Idle Thread

The TinyThreads system is designed with a fundamental assumption:

> **The main thread should be an idle thread that does not participate in cooperative scheduling.**

### Why This Design?

1. **Bootstrap Thread:** Main initializes the system and spawns worker threads
2. **No Context Required:** Idle loops don't need context preservation
3. **Stack Safety:** Avoids complexity of managing main's stack during context switches
4. **Separation of Concerns:** Worker threads handle computation; main handles initialization

### Architectural Pattern
```
main() {
    // Initialization phase
    piface_init();
    initTimerInterrupts();
    
    // Spawn worker threads
    spawn(worker1, arg1);
    spawn(worker2, arg2);
    spawn(worker3, arg3);
    spawn(worker4, arg4);
    
    // Become idle - let workers run
    while(1) {
        no_operation();  // or other non-yielding idle behavior
    }
}
```

## Solutions

### ✅ Solution 1: Spawn All Worker Threads (Recommended)
```c
int main() {
    piface_init();
    piface_puts("DT8025 - A4P1");
    RPI_WaitMicroSeconds(2000000);	
    piface_clear();
    initTimerInterrupts();
    
    spawn(computeSomethingForever, 0);
    spawn(computeSomethingForever, 1);
    spawn(computeSomethingForever, 2);
    spawn(computeSomethingForever, 3);
    
    // Main thread becomes idle loop
    while(1) {
        no_operation();
    }
}
```

### ⚠️ Solution 2: Run Finite Work in Main (Limited Use)
If you must run work in main, use a **finite** function and disable interrupts:
```c
int main() {
    // ... initialization ...
    spawn(computeSomethingForever, 0);
    spawn(computeSomethingForever, 1);
    spawn(computeSomethingForever, 2);
    
    DISABLE();  // Prevent context switches
    computeSomething(3);  // Finite version only!
    ENABLE();
    
    while(1) {
        no_operation();
    }
}
```
**⚠️ Warning:** This prevents preemption during main's work, defeating the purpose of real-time scheduling.

### ❌ Solution 3: Don't Do This
```c
// WRONG - Will crash!
spawn(computeSomethingForever, 0);
spawn(computeSomethingForever, 1);
spawn(computeSomethingForever, 2);
computeSomethingForever(3);  // ❌ Never returns, yields, causes stack corruption
```

## Key Takeaways

1. **Main thread (`initp`) is not designed to participate in thread scheduling**
2. **Only spawned threads have properly allocated stacks for context switching**
3. **Calling `yield()` from main causes stack corruption during context switches**
4. **Timer interrupts make the problem worse by forcing context switches at unpredictable times**
5. **The architectural pattern is: main initializes, spawns workers, then becomes idle**

## Related Code References

- `tinythreads.c:78-84` - `initp` initialization (missing stack setup)
- `tinythreads.c:152-172` - `spawn()` function (proper thread initialization)
- `tinythreads.c:177-185` - `yield()` function (enqueues current thread)
- `tinythreads.c:133-144` - `dispatch()` function (context switching)
- `tinythreads.c:44` - `SETSTACK` macro definition
- `rpi-interrupts.c:95-106` - Timer interrupt handler
- `a4p1.c:93-107` - `computeSomethingForever()` function (calls `yield()`)

## Conclusion

The failure when running `computeSomethingForever` in main is not a bug, but a violation of the TinyThreads architectural design. The system expects main to be an idle thread that doesn't participate in cooperative scheduling. By understanding this design principle and the underlying stack management issues, we can write correct and reliable real-time embedded applications.

**Remember:** In TinyThreads, main initializes and then gets out of the way. Let the spawned threads do the work!

