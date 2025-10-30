# Why You Cannot Call yield() from the Main Thread

## Quick Answer

**You cannot call `yield()` from the main thread because the main thread lacks a dedicated, isolated stack required for safe context switching.**

When `yield()` is called from main:
1. The main thread enters the ready queue (`readyQ`)
2. The scheduler attempts to context switch to/from main
3. `setjmp/longjmp` save/restore a stack pointer that references the **shared system stack**
4. Timer interrupts and other threads corrupt this shared stack
5. When context is restored, the stack contains invalid data
6. **Result:** Crashes, hangs, or undefined behavior

## Table of Contents
- [How Context Switching Works](#how-context-switching-works)
- [The Stack Pointer Problem](#the-stack-pointer-problem)
- [Why This Causes Crashes](#why-this-causes-crashes)
- [Visual Example of the Crash](#visual-example-of-the-crash)
- [Why no_operation() Loop is Safe](#why-no_operation-loop-is-safe)
- [Could We Make yield() Work in Main?](#could-we-make-yield-work-in-main)
- [Summary](#summary)

---

## How Context Switching Works

### The yield() Function

When a thread calls `yield()`, it voluntarily gives up the CPU:

```c
void yield(void) {
    DISABLE();
    if (readyQ != NULL){		
        thread p = dequeue(&readyQ);
        enqueue(current, &readyQ);  // ← Current thread goes to ready queue
        dispatch(p);                // ← Switch to next thread
    }	
    ENABLE();
}
```

**Flow:**
1. Dequeue the next thread from `readyQ`
2. Enqueue the current thread to `readyQ` (to run again later)
3. Call `dispatch()` to perform the context switch

### The dispatch() Function

The actual context switch happens in `dispatch()`:

```c
static void dispatch(thread next) {
    if (next != NULL) {
        if (setjmp(current->context) == 0) {  // ← Save current context
            current = next;
            longjmp(next->context, 1);        // ← Jump to next context
        }
    }
}
```

**What happens:**
1. `setjmp(current->context)` saves the **entire CPU state** including:
   - All registers (r0-r15)
   - **Stack pointer (SP)** ← Critical!
   - Program counter (PC)
   - CPU status flags
2. `longjmp(next->context, 1)` restores the next thread's saved state
3. Execution continues from where the next thread previously called `setjmp`

### What setjmp/longjmp Actually Do

```c
// Simplified representation
int setjmp(jmp_buf env) {
    env[0] = r0;
    env[1] = r1;
    // ... save all registers ...
    env[8] = SP;   // ← Stack Pointer saved here!
    env[9] = PC;   // Program Counter
    // ...
    return 0;      // First return
}

void longjmp(jmp_buf env, int val) {
    r0 = env[0];
    r1 = env[1];
    // ... restore all registers ...
    SP = env[8];   // ← Stack Pointer restored!
    PC = env[9];
    // ...
    return val;    // "Returns" to where setjmp was called
}
```

**The critical point:** The stack pointer (SP) is saved and restored. This SP must point to a **valid, isolated stack** for the thread.

---

## The Stack Pointer Problem

### Spawned Threads: Dedicated Stack (Correct)

When you call `spawn()` to create a thread, it allocates a dedicated stack:

```c
void spawn(void (* function)(int), int arg) {
    thread newp;
    DISABLE();
    if (!initialized) 
        initialize();
    newp = dequeue(&freeQ);
    newp->function = function;
    newp->arg = arg;
    newp->next = NULL;
    if (setjmp(newp->context) == 1) {
        ENABLE();
        current->function(current->arg);
        // ...
    }
    SETSTACK(&newp->context, &newp->stack);  // ← KEY LINE!
    enqueue(newp, &readyQ);
    ENABLE();
}
```

**The SETSTACK macro:**
```c
#define SETSTACK(buf,a) *((unsigned int *)(buf)+8) = (unsigned int)(a) + STACKSIZE - 4;
```

This sets the stack pointer (at position 8 in the context buffer) to point to the thread's dedicated stack.

#### Memory Layout for Spawned Thread

```
Thread Structure (thread_block):
┌─────────────────────────────────┐
│ idx: 0                          │
│ function: computeSomethingForever│
│ arg: 0                          │
│ next: NULL                      │
│ context: jmp_buf                │ ← Context buffer
│   [0] = r0                      │
│   [1] = r1                      │
│   ...                           │
│   [8] = SP ───────────┐         │
│   [9] = PC            │         │
│ stack[1024 bytes] ←───┘         │ ← Dedicated, isolated stack
│   [0]                           │
│   [1]                           │
│   ...                           │
│   [1023]                        │
│ Period_Deadline: ...            │
└─────────────────────────────────┘
```

**Result:** 
- ✅ Each thread has its own isolated 1024-byte stack
- ✅ Stack pointer points to thread's own stack
- ✅ No interference between threads
- ✅ Safe for context switching

### Main Thread: No Dedicated Stack (Problem!)

The main thread (`initp`) is initialized differently:

```c
void initialize(void) {
    initp.idx = -1;
    initp.function = NULL;
    initp.arg = -1;
    initp.next = NULL;
    initp.Period_Deadline = INT_MAX;
    initp.Rel_Period_Deadline = INT_MAX;	
    
    // ❌ NO SETSTACK CALLED!
    // ❌ NO stack allocation!
    // ❌ initp.stack doesn't even exist in the structure!
}
```

Looking at the thread_block structure definition:

```c
struct thread_block threads[NTHREADS];  // Array of 5 threads
struct thread_block initp;              // ← Separate, no stack array!
```

**The initp structure has:**
- ❌ No `char stack[STACKSIZE]` field
- ❌ No `SETSTACK()` call
- ❌ No dedicated stack allocation

#### Memory Layout for Main Thread

```
initp Structure:
┌─────────────────────────────────┐
│ idx: -1                         │
│ function: NULL                  │
│ arg: -1                         │
│ next: NULL                      │
│ context: jmp_buf                │
│   [0] = r0                      │
│   [1] = r1                      │
│   ...                           │
│   [8] = SP ───────────┐         │ ← Points to system stack!
│   [9] = PC            │         │
│ ❌ NO stack field!    │         │
│ Period_Deadline: ... │         │
└──────────────────────┼─────────┘
                       │
                       └─────────→ System Main Stack
                                   ┌──────────────────┐
                                   │ Startup code     │
                                   │ main() frame     │
                                   │ Local variables  │
                                   │ Function calls   │
                                   └──────────────────┘
```

**Result:**
- ❌ Main thread uses the **shared system stack**
- ❌ Stack pointer points to whatever the system SP currently is
- ❌ Not isolated from interrupts and initialization code
- ❌ **Unsafe for context switching**

---

## Why This Causes Crashes

### Problem 1: Stack Reuse and Corruption

When the scheduler switches contexts while main is in the ready queue:

#### Timeline of Stack Corruption

```
Time T0: Main thread running computeSomethingForever(3)
========================================================
System Stack:
┌─────────────────────────────────┐
│ main() local variables          │  0x8000
│ computeSomethingForever() frame │  0x7F00
│   - int seg = 3                 │  0x7EFC
│   - ExpStruct* value            │  0x7EF8
│   - uint32_t i                  │  0x7EF4
│   - return address              │  0x7EF0  ← SP = 0x7EF0
└─────────────────────────────────┘

Time T1: Main thread calls yield()
========================================================
Action: setjmp(initp->context) saves SP = 0x7EF0

System Stack: (unchanged)
┌─────────────────────────────────┐
│ main() local variables          │  0x8000
│ computeSomethingForever() frame │  0x7F00
│   - int seg = 3                 │  0x7EFC
│   - ExpStruct* value            │  0x7EF8
│   - uint32_t i                  │  0x7EF4
│   - return address              │  0x7EF0  ← Saved as SP
└─────────────────────────────────┘

initp->context[8] = 0x7EF0  ← Saved!

Time T2: Context switch to Thread 1
========================================================
Thread 1 is now running, using its own dedicated stack (0x2000-0x2400)
System stack is not being used... or is it?

Time T3: Timer interrupt fires!
========================================================
💥 Interrupt handler uses the system stack!

IRQ Handler pushes onto system stack:
┌─────────────────────────────────┐
│ main() local variables          │  0x8000
│ computeSomethingForever() frame │  0x7F00
│   - CORRUPTED DATA              │  0x7EFC  ← Overwritten!
│   - CORRUPTED DATA              │  0x7EF8  ← Overwritten!
│   - IRQ return address          │  0x7EF4  ← NEW!
│   - IRQ saved registers         │  0x7EF0  ← NEW!
│   - IRQ stack frame             │  0x7EE0  ← NEW!
└─────────────────────────────────┘

The original data at 0x7EF0-0x7EFC is now CORRUPTED!

Time T4: Interrupt handler calls scheduler()
========================================================
scheduler() → scheduler_RR() → dispatch(initp)

Time T5: longjmp(initp->context) restores SP = 0x7EF0
========================================================
💥 CPU restores SP = 0x7EF0, expecting to find:
   - computeSomethingForever's local variables
   - Valid return address

But actually finds:
   - IRQ handler's garbage data!
   - Invalid return address!

When function tries to return:
   - Reads return address from stack
   - Gets corrupted value (IRQ data)
   - Jumps to invalid memory location
   💥 CRASH!
```

### Problem 2: Non-Isolated Stack Space

**Spawned Threads** have isolated stacks:

```
Memory Layout:
┌─────────────────────────────────┐
│ Thread 0: stack[0..1023]        │  0x2000-0x2400
│   ✅ Isolated from others       │
├─────────────────────────────────┤
│ Thread 1: stack[0..1023]        │  0x2400-0x2800
│   ✅ Isolated from others       │
├─────────────────────────────────┤
│ Thread 2: stack[0..1023]        │  0x2800-0x2C00
│   ✅ Isolated from others       │
├─────────────────────────────────┤
│ Thread 3: stack[0..1023]        │  0x2C00-0x3000
│   ✅ Isolated from others       │
└─────────────────────────────────┘
```

When Thread 1 runs:
- Uses memory 0x2400-0x2800 **only**
- Cannot corrupt Thread 0's stack (0x2000-0x2400)
- Cannot corrupt Thread 2's stack (0x2800-0x2C00)
- ✅ **Safe isolation**

**Main Thread** has shared system stack:

```
Memory Layout:
┌─────────────────────────────────┐
│ System Stack (shared)           │  0x7000-0x8000
│   Used by:                      │
│   - Boot/startup code           │
│   - main() initialization       │
│   - Timer interrupt handlers!   │  ← Multiple users!
│   - Main thread execution       │
│   ❌ NOT ISOLATED!              │
└─────────────────────────────────┘
```

When interrupt fires:
- IRQ handler **pushes onto system stack**
- **Overwrites main thread's data**
- ❌ **No isolation, guaranteed corruption**

### Problem 3: Interrupt Stack Mode Conflicts

ARM processors have different stack modes:

```
ARM Stack Modes:
┌─────────────────┬──────────────────┬────────────────┐
│ Mode            │ Stack Used       │ Stack Pointer  │
├─────────────────┼──────────────────┼────────────────┤
│ User/System     │ Main stack       │ SP             │
│ IRQ (Interrupt) │ IRQ stack        │ SP_irq         │
│ FIQ             │ FIQ stack        │ SP_fiq         │
│ Supervisor      │ SVC stack        │ SP_svc         │
│ Abort           │ Abort stack      │ SP_abt         │
│ Undefined       │ Undefined stack  │ SP_und         │
└─────────────────┴──────────────────┴────────────────┘
```

**What happens during an interrupt:**

```
Before interrupt: Main thread running in User mode
┌────────────────────────────────┐
│ CPU Mode: User                 │
│ Stack: SP = 0x7EF0             │
│ Executing: computeSomething... │
└────────────────────────────────┘

Interrupt fires:
┌────────────────────────────────┐
│ 1. CPU saves return address    │
│ 2. CPU switches to IRQ mode    │
│ 3. SP_irq becomes active       │
│ 4. Pushes context to IRQ stack │
└────────────────────────────────┘

During interrupt handler:
┌────────────────────────────────┐
│ CPU Mode: IRQ                  │
│ Stack: SP_irq (different!)     │
│ But: May also use main stack!  │  ← Problem!
└────────────────────────────────┘
```

**The conflict:**
- Main thread's saved context points to User mode stack (SP = 0x7EF0)
- Interrupt handler might push data to the same area
- When returning to main thread, stack is corrupted
- Different stack modes share the same physical memory!

### Problem 4: Multiple Context Saves Overwrite Each Other

Each time `yield()` is called, `setjmp` saves the **current** stack pointer:

```
First call to yield() from main:
  Stack depth: 5 function calls deep
  SP = 0x7F00
  setjmp saves: context[8] = 0x7F00

Main runs again after context switch:
  Calls another function
  Stack depth: 6 function calls deep (deeper!)
  SP = 0x7EE0  ← Different!

Second call to yield() from main:
  setjmp saves: context[8] = 0x7EE0  ← Overwrites previous!

Later, longjmp(initp->context):
  Restores SP = 0x7EE0
  But stack contents might be from when SP was 0x7F00!
  💥 Mismatch between SP and actual stack data!
```

**Spawned threads don't have this problem** because:
- Stack pointer is set **once** by `SETSTACK` to: `stack + STACKSIZE - 4`
- Always starts from the **same, known location**
- Context switches don't change the base stack location
- ✅ Consistent and predictable

---

## Visual Example of the Crash

Let's trace a specific crash scenario:

### Scenario Setup

```c
int main() {
    initTimerInterrupts();
    spawn(computeSomethingForever, 0);
    spawn(computeSomethingForever, 1);
    spawn(computeSomethingForever, 2);
    computeSomethingForever(3);  // ❌ Running in main!
}

void computeSomethingForever(int seg) {
    ExpStruct* value;
    for(volatile uint32_t i=0; ; i++) {
        value = iexp((i % 9) + 1);
        printf_at_seg(seg % 4, "T%i: %d", seg, value->expInt);
        free(value);
        yield();  // ← Main calls this!
    }
}
```

### Step-by-Step Crash

```
STEP 1: Main thread starts computeSomethingForever(3)
─────────────────────────────────────────────────────
Stack (grows downward):
0x8000  ┌─────────────────────────┐
        │ main() stack frame      │
0x7F80  ├─────────────────────────┤
        │ computeSomethingForever │
        │   seg = 3               │
        │   value = 0x00000000    │
        │   i = 0                 │
        │   return addr = 0x8120  │
0x7F00  └─────────────────────────┘ ← SP

STEP 2: First iteration, calls iexp()
─────────────────────────────────────────────────────
0x8000  ┌─────────────────────────┐
        │ main() stack frame      │
0x7F80  ├─────────────────────────┤
        │ computeSomethingForever │
        │   seg = 3               │
        │   value = 0x00000000    │
        │   i = 0                 │
        │   return addr = 0x8120  │
0x7F00  ├─────────────────────────┤
        │ iexp() stack frame      │
        │   locals...             │
        │   return addr = 0x7F50  │
0x7E80  └─────────────────────────┘ ← SP

STEP 3: Returns from iexp(), calls yield()
─────────────────────────────────────────────────────
0x8000  ┌─────────────────────────┐
        │ main() stack frame      │
0x7F80  ├─────────────────────────┤
        │ computeSomethingForever │
        │   seg = 3               │
        │   value = 0xABCD1234    │ ← Now has value
        │   i = 0                 │
        │   return addr = 0x8120  │
0x7F00  ├─────────────────────────┤
        │ yield() stack frame     │
        │   return addr = 0x7F40  │
0x7EF0  └─────────────────────────┘ ← SP

Action: setjmp(initp->context) saves SP = 0x7EF0

STEP 4: Context switch to Thread 1
─────────────────────────────────────────────────────
Thread 1 is now running
System stack unchanged (not being used)

STEP 5: Timer interrupt fires during Thread 1
─────────────────────────────────────────────────────
CPU switches to IRQ mode
Interrupt handler pushes onto system stack!

0x8000  ┌─────────────────────────┐
        │ main() stack frame      │
0x7F80  ├─────────────────────────┤
        │ computeSomethingForever │
        │   seg = 3               │
        │   value = 0xABCD1234    │
        │   i = 0                 │
        │   return addr = 0x8120  │
0x7F00  ├─────────────────────────┤
        │ ❌ CORRUPTED DATA       │ ← Was yield() frame
        │ ❌ CORRUPTED = 0x????   │ ← Was return address
0x7EF0  ├─────────────────────────┤
        │ IRQ saved registers     │ ← NEW! Overwrote data
        │ IRQ return address      │
        │ scheduler() frame       │
0x7E00  └─────────────────────────┘ ← SP_irq

STEP 6: Interrupt handler calls scheduler()
─────────────────────────────────────────────────────
scheduler() → scheduler_RR() → dispatch()
Decides to switch back to main thread (initp)

STEP 7: longjmp(initp->context) restores context
─────────────────────────────────────────────────────
Action: Restores SP = 0x7EF0 (saved in Step 3)

CPU state:
  SP = 0x7EF0
  PC = address after yield() call

Stack at 0x7EF0:
0x7EF0  ┌─────────────────────────┐
        │ ❌ IRQ garbage data     │ ← Expects yield() return addr!
        │ ❌ Corrupted values     │
        └─────────────────────────┘

STEP 8: yield() tries to return
─────────────────────────────────────────────────────
Code executes: return from yield()

CPU action:
1. Pops return address from stack at SP (0x7EF0)
2. Expects 0x7F40 (return to computeSomethingForever)
3. Actually gets 0x???? (IRQ garbage!)
4. Jumps to 0x????

💥 CRASH! Invalid instruction or memory access
```

### Why Spawned Threads Don't Crash

```
Thread 1 running:
Stack: 0x3000-0x3400 (dedicated)

Timer interrupt fires:
IRQ uses: 0x7E00 (system IRQ stack, different!)

Thread 1's stack: UNTOUCHED ✅
  0x3000-0x3400 remains intact

longjmp back to Thread 1:
  Restores SP = 0x3350 (or wherever it was)
  Stack data at 0x3350: VALID ✅
  Return address: VALID ✅
  ✅ NO CRASH!
```

---

## Why no_operation() Loop is Safe

The recommended pattern:

```c
int main() {
    piface_init();
    initTimerInterrupts();
    spawn(computeSomethingForever, 0);
    spawn(computeSomethingForever, 1);
    spawn(computeSomethingForever, 2);
    spawn(computeSomethingForever, 3);
    
    // Main thread becomes idle loop
    while(1) {
        no_operation();  // Just executes NOP instruction
    }
}
```

### Why This Works

#### 1. No yield() Calls
```c
void no_operation() {
    __asm volatile("nop \n");  // Just a single NOP instruction
}
```

- ✅ Never calls `yield()`
- ✅ Main thread (`initp`) never enters `readyQ`
- ✅ No context switching to/from main
- ✅ No `setjmp/longjmp` on main's context

#### 2. Scheduler Ignores Main Thread

When timer interrupt fires:

```c
void interrupt_vector(void) {
    if( RPI_GetArmTimer()->MaskedIRQ ) {
        RPI_GetArmTimer()->IRQClear = 1;
        ticks++;
        scheduler();  // ← Calls scheduler
    }
}

static void scheduler_RR(void){
    if(readyQ != NULL){
        thread p = dequeue(&readyQ);
        enqueue(current, &readyQ);
        dispatch(p);
    }
}
```

**readyQ contains:**
```
readyQ: Thread0 → Thread1 → Thread2 → Thread3 → NULL
        ✅ All have dedicated stacks
        ✅ All properly initialized
        ❌ initp is NOT in this queue!
```

**Execution flow:**
1. Interrupt fires while Thread 2 is running
2. `current` = Thread 2
3. Dequeue Thread 3 from readyQ
4. Enqueue Thread 2 to readyQ
5. Dispatch to Thread 3
6. **Main thread not involved!** ✅

#### 3. Main Thread Stays in Simple Loop

```
Main thread execution (simplified):
┌─────────────────────────────────┐
│ while(1) {                      │
│     nop;      ←─────────────┐   │
│ }             ──────────────┘   │
└─────────────────────────────────┘
    Infinite loop, never exits
    Never calls yield()
    Never participates in scheduling
```

**Stack usage:**
```
System Stack:
0x8000  ┌─────────────────────────┐
        │ main() stack frame      │
        │   - very simple         │
        │   - never changes       │
0x7FF0  └─────────────────────────┘ ← SP stays constant

No function calls (except nop inline asm)
No stack growth
No corruption possible ✅
```

#### 4. Interrupts Don't Affect Main

When interrupt fires while main is in no-op loop:

```
Before interrupt:
  Main thread: while(1) { nop; }
  SP = 0x7FF0

Interrupt fires:
  CPU switches to IRQ mode
  SP_irq is used (different from SP!)
  Scheduler switches between Thread 0-3
  
After interrupt returns:
  CPU returns to User mode
  Main thread: while(1) { nop; }
  SP = 0x7FF0  ← Still the same! ✅
  
Main thread never knows interrupt happened!
```

### Comparison Table

| Aspect | no_operation() Loop | computeSomethingForever() in Main |
|--------|---------------------|-----------------------------------|
| **Calls yield()** | ❌ No | ✅ Yes → Problem! |
| **Enters readyQ** | ❌ No | ✅ Yes → Problem! |
| **Context switches** | ❌ No | ✅ Yes → Problem! |
| **Stack changes** | ❌ Constant | ✅ Dynamic → Problem! |
| **Interrupt safe** | ✅ Yes | ❌ No |
| **Result** | ✅ Works perfectly | 💥 Crashes |

---

## Could We Make yield() Work in Main?

### Theoretical Solution

Yes, it's technically possible to make main participate in scheduling, but it would require:

#### Required Changes

1. **Add stack field to initp:**
```c
struct thread_block initp = {
    .idx = -1,
    .function = NULL,
    .arg = -1,
    .next = NULL,
    .stack = {0},  // ← Would need to add this!
    .Period_Deadline = INT_MAX,
    .Rel_Period_Deadline = INT_MAX
};
```

2. **Call SETSTACK for initp:**
```c
void initialize(void) {
    initp.idx = -1;
    initp.function = NULL;
    initp.arg = -1;
    initp.next = NULL;
    
    // ✅ Add this:
    SETSTACK(&initp.context, &initp.stack);
    
    initp.Period_Deadline = INT_MAX;
    initp.Rel_Period_Deadline = INT_MAX;
}
```

3. **Carefully transition from system stack to thread stack:**
```c
int main() {
    // Running on system stack
    piface_init();
    initTimerInterrupts();
    
    // Need to switch to initp's dedicated stack before spawning!
    // This is complex and error-prone...
    
    // Then can safely call:
    spawn(computeSomethingForever, 0);
    spawn(computeSomethingForever, 1);
    spawn(computeSomethingForever, 2);
    computeSomethingForever(3);  // Now safe (maybe)
}
```

### Why This is a Bad Idea

#### 1. Architectural Complexity
- Adds complexity for minimal benefit
- Stack transition is error-prone
- Violates single-responsibility principle
  - Main should initialize
  - Worker threads should work

#### 2. Bootstrap Problem
```
Question: When does main switch to its dedicated stack?

Option A: Before any initialization
  ❌ Initialization code expects system stack
  ❌ Library functions expect system stack
  ❌ May break assumptions

Option B: After initialization
  ❌ How to switch stacks mid-execution?
  ❌ Requires assembly code
  ❌ Stack contains return addresses to system stack functions!
```

#### 3. Design Philosophy Violation

The TinyThreads architecture follows a clear pattern:

```
┌──────────────────────────────────────┐
│         Bootstrap Thread             │
│              (main)                  │
│                                      │
│  Responsibilities:                   │
│  - Initialize hardware               │
│  - Create worker threads             │
│  - Become idle                       │
│  - Let workers do the work          │
└──────────────────────────────────────┘
              │
              ├─────────────┬──────────────┬───────────────┐
              ▼             ▼              ▼               ▼
       ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐
       │ Thread 0 │  │ Thread 1 │  │ Thread 2 │  │ Thread 3 │
       │          │  │          │  │          │  │          │
       │ Worker   │  │ Worker   │  │ Worker   │  │ Worker   │
       └──────────┘  └──────────┘  └──────────┘  └──────────┘
```

**This is a good design because:**
- Clear separation of concerns
- Main is simple and predictable
- Workers are isolated and schedulable
- Easy to reason about
- Follows embedded systems best practices

#### 4. Unnecessary Effort

You have **5 thread slots** available:
```c
#define NTHREADS 5
struct thread_block threads[NTHREADS];
```

So you can spawn **5 worker threads**:
```c
spawn(computeSomethingForever, 0);
spawn(computeSomethingForever, 1);
spawn(computeSomethingForever, 2);
spawn(computeSomethingForever, 3);
spawn(computeSomethingForever, 4);  // 5th thread!
```

**Why make main a worker when you have 5 thread slots?**
- No benefit to using main
- Only adds complexity and risk
- Just spawn all workers and keep main simple!

### Correct Approach: Keep It Simple

```c
int main() {
    // Initialize (running on system stack - fine!)
    piface_init();
    piface_puts("DT8025 - A4P1");
    RPI_WaitMicroSeconds(2000000);	
    piface_clear();
    initTimerInterrupts();
    
    // Create worker threads (each gets dedicated stack)
    spawn(computeSomethingForever, 0);  // ✅ Thread 0
    spawn(computeSomethingForever, 1);  // ✅ Thread 1
    spawn(computeSomethingForever, 2);  // ✅ Thread 2
    spawn(computeSomethingForever, 3);  // ✅ Thread 3
    
    // Become idle (stay on system stack - fine!)
    while(1) {
        no_operation();  // ✅ Never yields, never switches
    }
    
    // Clean, simple, safe! ✅
}
```

---

## Summary

### The Core Issue

**You cannot call `yield()` from the main thread because:**

1. **No Dedicated Stack**
   - Spawned threads: Have `char stack[STACKSIZE]` allocated
   - Main thread: Uses shared system stack
   - `SETSTACK` never called for main thread

2. **Stack Pointer Corruption**
   - `setjmp` saves current SP from system stack
   - System stack is shared with interrupts and initialization
   - Timer interrupts overwrite saved stack contents
   - `longjmp` restores corrupted SP → crash

3. **Interrupt Conflicts**
   - IRQ handlers push frames onto system stack
   - Main thread's saved context references same memory
   - Context restoration finds garbage data
   - Return addresses corrupted → undefined behavior

4. **Design Violation**
   - TinyThreads expects main to be bootstrap/idle thread
   - Worker threads should be spawned with dedicated resources
   - Main should initialize, spawn workers, then stay idle

### The Solution

✅ **Always use this pattern:**
```c
int main() {
    // Initialize
    initialize_hardware();
    initTimerInterrupts();
    
    // Spawn all workers
    spawn(worker_function, 0);
    spawn(worker_function, 1);
    spawn(worker_function, 2);
    spawn(worker_function, 3);
    
    // Become idle
    while(1) {
        no_operation();
    }
}
```

### Key Rules

1. ✅ **DO:** Spawn threads for all work that needs to yield
2. ✅ **DO:** Keep main as a simple idle loop
3. ✅ **DO:** Use `no_operation()` or simple non-yielding code in main
4. ❌ **DON'T:** Call `yield()` from main
5. ❌ **DON'T:** Call functions that call `yield()` from main
6. ❌ **DON'T:** Try to make main participate in scheduling

### Memory Layout Recap

```
Correct (Safe):
┌────────────────────────────────────┐
│ Main Thread (initp)                │
│ - Uses system stack                │
│ - Never yields                     │
│ - Never context switches           │
│ - Simple idle loop                 │
└────────────────────────────────────┘

┌─────────┬─────────┬─────────┬─────────┐
│Thread 0 │Thread 1 │Thread 2 │Thread 3 │
│Stack    │Stack    │Stack    │Stack    │
│1024B    │1024B    │1024B    │1024B    │
│✅ Safe  │✅ Safe  │✅ Safe  │✅ Safe  │
└─────────┴─────────┴─────────┴─────────┘
```

```
Incorrect (Crashes):
┌────────────────────────────────────┐
│ Main Thread (initp)                │
│ - Uses system stack ❌             │
│ - Calls yield() ❌                 │
│ - Enters readyQ ❌                 │
│ - Context switches with bad SP ❌  │
│ - Stack corruption ❌              │
│ 💥 CRASH!                          │
└────────────────────────────────────┘
```

### Final Takeaway

> **The main thread in TinyThreads is the bootstrap thread. It initializes the system, spawns worker threads, and then stays out of the way. Never call `yield()` from main.**

This is not a limitation—it's a design feature that keeps the system simple, predictable, and safe!

---

## References

- `tinythreads.c:78-96` - `initialize()` function (initp initialization)
- `tinythreads.c:152-172` - `spawn()` function (thread creation with SETSTACK)
- `tinythreads.c:177-185` - `yield()` function (cooperative scheduling)
- `tinythreads.c:133-144` - `dispatch()` function (setjmp/longjmp)
- `tinythreads.c:44` - `SETSTACK` macro definition
- `tinythreads.c:49-58` - `thread_block` structure definition
- `rpi-interrupts.c:95-106` - `interrupt_vector()` (timer IRQ handler)
- `a4p1.c:93-107` - `computeSomethingForever()` (calls yield)
- `a4p1.c:119-134` - `main()` function (correct implementation)

