# TinyThreads: Animated Step-by-Step Walkthrough

This document provides frame-by-frame animation of TinyThreads execution using ASCII art. Each "frame" shows the system state at a specific moment in time.

---

## Animation 1: System Initialization

### Frame 1: Power On
```
┌────────────────────────────────────────────────────────┐
│                    SYSTEM BOOT                         │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Raspberry Pi 3 Starting...                            │
│  ARMv8 CPU: Online                                     │
│  Memory: Initialized                                   │
│                                                        │
│  TinyThreads State:                                    │
│  ┌──────────────────────────────────────────────┐    │
│  │ freeQ:       [uninitialized]                  │    │
│  │ readyQ:      NULL                             │    │
│  │ doneQ:       NULL                             │    │
│  │ current:     &initp (main)                    │    │
│  │ initialized: 0                                │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  Thread Pool:                                          │
│  ┌──────────────────────────────────────────────┐    │
│  │ [???][???][???][???][???]                       │    │
│  │  T0   T1   T2   T3   T4   (garbage data)      │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 2: piface_init() and Display
```
┌────────────────────────────────────────────────────────┐
│              PIFACE INITIALIZATION                     │
├────────────────────────────────────────────────────────┤
│                                                        │
│  ╔═══════════════════════════╗                        │
│  ║  PiFace LCD Display       ║                        │
│  ║  ┌───────────────────┐   ║                        │
│  ║  │ DT8025 - A3P1     │   ║  ← Displayed 2 seconds │
│  ║  │                   │   ║                        │
│  ║  └───────────────────┘   ║                        │
│  ╚═══════════════════════════╝                        │
│                                                        │
│  Code executing:                                       │
│  ┌──────────────────────────────────────────────┐    │
│  │ piface_init();                                │    │
│  │ piface_clear();                               │    │
│  │ piface_puts("DT8025 - A3P1");                │    │
│  │ RPI_WaitMicroSeconds(2000000);  ← HERE        │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 3: First spawn() Call - Before initializeThreads()
```
┌────────────────────────────────────────────────────────┐
│          spawn(computePower, 0) - STEP 1               │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Code Path:                                            │
│  ┌──────────────────────────────────────────────┐    │
│  │ void spawn(void (*f)(int), int arg) {         │    │
│  │     thread newp;                              │    │
│  │     DISABLE();           ← INTERRUPTS OFF     │    │
│  │     if (!initialized)    ← TRUE (0)           │    │
│  │         initializeThreads();  ← CALLING...    │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  CPU State:                                            │
│  ┌──────────────────────────────────────────────┐    │
│  │ PRIMASK: 1  (interrupts disabled)             │    │
│  │ PC:      Inside spawn()                       │    │
│  │ SP:      Main's stack                         │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 4: initializeThreads() Execution
```
┌────────────────────────────────────────────────────────┐
│           INITIALIZING THREAD POOL                     │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Loop Progress: for (i=0; i < NTHREADS; i++)           │
│                                                        │
│  Thread 0:  [✓] idx=0, next→T1, function=NULL         │
│  Thread 1:  [✓] idx=1, next→T2, function=NULL         │
│  Thread 2:  [✓] idx=2, next→T3, function=NULL         │
│  Thread 3:  [✓] idx=3, next→T4, function=NULL         │
│  Thread 4:  [✓] idx=4, next=NULL, function=NULL       │
│                                                        │
│  Linked List Formation:                                │
│  ┌────────────────────────────────────────────────┐  │
│  │                                                 │  │
│  │  freeQ ──▶ [T0] ──▶ [T1] ──▶ [T2] ──▶ [T3] ──▶│  │
│  │                                                 │  │
│  │            [T4] ──▶ NULL                        │  │
│  │             ▲                                   │  │
│  │             └─────────────────────────────────  │  │
│  │                                                 │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  Final Setup:                                          │
│  ┌──────────────────────────────────────────────┐    │
│  │ freeQ = &threads[0];   ← Points to Thread 0   │    │
│  │ initialized = 1;       ← Flag set             │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 5: Allocating Thread 0
```
┌────────────────────────────────────────────────────────┐
│        spawn() Continues - Allocate Thread             │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Code:                                                 │
│  ┌──────────────────────────────────────────────┐    │
│  │ newp = dequeue(&freeQ);  ← EXECUTING          │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  BEFORE dequeue:                                       │
│  ┌────────────────────────────────────────────────┐  │
│  │ freeQ ──▶ [T0] ──▶ [T1] ──▶ [T2] ──▶ [T3] ──▶ │  │
│  │                    [T4] ──▶ NULL               │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  AFTER dequeue:                                        │
│  ┌────────────────────────────────────────────────┐  │
│  │ freeQ ──▶ [T1] ──▶ [T2] ──▶ [T3] ──▶ [T4] ──▶ │  │
│  │           NULL                                  │  │
│  │                                                 │  │
│  │ newp  ──▶ [T0]  (removed from queue)           │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  Thread 0 Status:                                      │
│  ┌──────────────────────────────────────────────┐    │
│  │ idx:      0                                    │    │
│  │ function: NULL  ← About to be set              │    │
│  │ arg:      -1    ← About to be set              │    │
│  │ next:     NULL  ← Isolated                     │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 6: Configuring Thread 0
```
┌────────────────────────────────────────────────────────┐
│      Configuring Thread 0 for computePower             │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Code Execution:                                       │
│  ┌──────────────────────────────────────────────┐    │
│  │ newp->function = function;  ← computePower    │    │
│  │ newp->arg = arg;            ← 0               │    │
│  │ newp->next = NULL;                            │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  Thread 0 After Configuration:                         │
│  ┌──────────────────────────────────────────────┐    │
│  │                   ┌─────────────┐             │    │
│  │ idx:       0      │ THREAD 0    │             │    │
│  │ function:  ──────▶│ computePower│             │    │
│  │ arg:       0      │   (code)    │             │    │
│  │ next:      NULL   └─────────────┘             │    │
│  │ context:   [not yet initialized]              │    │
│  │ stack:     [1024 bytes ready]                 │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  ⚠ IMPORTANT: Thread not yet ready to run!            │
│     Must setup context and stack first.               │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 7: Context Setup (setjmp)
```
┌────────────────────────────────────────────────────────┐
│          Setting Up Thread 0 Context                   │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Code:                                                 │
│  ┌──────────────────────────────────────────────┐    │
│  │ if (setjmp(newp->context) == 1) {             │    │
│  │     // Thread start code (skipped for now)    │    │
│  │ }                                             │    │
│  │ // Returns 0 on first call, so skip if-block │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  What setjmp() Saves (into newp->context):             │
│  ┌────────────────────────────────────────────────┐  │
│  │ Register      │ Value                          │  │
│  ├───────────────┼────────────────────────────────┤  │
│  │ PC (x30/LR)   │ Return address in spawn()      │  │
│  │ SP (x31)      │ Current stack pointer          │  │
│  │ x0-x28        │ General purpose registers      │  │
│  │ FP (x29)      │ Frame pointer                  │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  Result: Returns 0 (save operation completed)          │
│                                                        │
│  Next: Need to modify saved SP to use thread's stack  │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 8: Stack Setup (SETSTACK)
```
┌────────────────────────────────────────────────────────┐
│           Setting Thread 0's Stack Pointer             │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Macro:                                                │
│  ┌──────────────────────────────────────────────┐    │
│  │ #define SETSTACK(buf,a)                       │    │
│  │   *((unsigned int*)(buf)+8) =                 │    │
│  │     (unsigned int)(a) + STACKSIZE - 4;        │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  Execution:                                            │
│  ┌──────────────────────────────────────────────┐    │
│  │ SETSTACK(&newp->context, &newp->stack);       │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  Thread 0 Memory Layout:                               │
│  ┌────────────────────────────────────────────────┐  │
│  │                                                 │  │
│  │  High Address  ┌─────────────┐  ← stack[1023] │  │
│  │                │             │                  │  │
│  │                │   STACK     │  ← SP now points│  │
│  │                │   (1024 B)  │    here (top)   │  │
│  │                │             │                  │  │
│  │                │      ▼      │                  │  │
│  │                │   (grows    │                  │  │
│  │                │    down)    │                  │  │
│  │  Low Address   └─────────────┘  ← stack[0]     │  │
│  │                                                 │  │
│  │  Context saved with:                            │  │
│  │  SP = &stack[1023]                              │  │
│  │  PC = inside spawn's if-block (line 172)        │  │
│  │                                                 │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 9: Enqueue to readyQ
```
┌────────────────────────────────────────────────────────┐
│        Adding Thread 0 to Ready Queue                  │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Code:                                                 │
│  ┌──────────────────────────────────────────────┐    │
│  │ enqueue(newp, &readyQ);                       │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  BEFORE enqueue:                                       │
│  ┌────────────────────────────────────────────────┐  │
│  │ readyQ ──▶ NULL                                 │  │
│  │                                                 │  │
│  │ newp   ──▶ [T0] (function=computePower, arg=0) │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  AFTER enqueue:                                        │
│  ┌────────────────────────────────────────────────┐  │
│  │ readyQ ──▶ [T0] ──▶ NULL                       │  │
│  │                                                 │  │
│  │ Thread 0 is now ready to be dispatched!         │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  Complete System State:                                │
│  ┌────────────────────────────────────────────────┐  │
│  │ freeQ:   [T1]→[T2]→[T3]→[T4]→NULL             │  │
│  │ readyQ:  [T0]→NULL               ✓ NEW         │  │
│  │ doneQ:   NULL                                   │  │
│  │ current: &initp (main thread)                   │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │ ENABLE();  ← Re-enable interrupts             │    │
│  │ // spawn() returns to main()                  │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

---

## Animation 2: First Context Switch

### Frame 10: computePrimes Starts
```
┌────────────────────────────────────────────────────────┐
│         Main Thread: computePrimes(1)                  │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Code:                                                 │
│  ┌──────────────────────────────────────────────┐    │
│  │ void computePrimes(int seg) {                 │    │
│  │     for(int n = 0; ; n++) {                   │    │
│  │         if (is_prime(n)) {                    │    │
│  │             PUTTOLDC("T%d: Prime %d", seg, n);│    │
│  │             RPI_WaitMicroSeconds(500000);     │    │
│  │             yield();                          │    │
│  │         }                                     │    │
│  │     }                                         │    │
│  │ }                                             │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  Loop Iterations:                                      │
│  ┌────────────────────────────────────────────────┐  │
│  │ n=0: is_prime(0) = 0  → skip                   │  │
│  │ n=1: is_prime(1) = 0  → skip                   │  │
│  │ n=2: is_prime(2) = 1  → FOUND!                 │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  ╔═══════════════════════════╗                        │
│  ║  PiFace LCD Display       ║                        │
│  ║  ┌───────────────────┐   ║                        │
│  ║  │ T1: Prime 2       │   ║  ← Displayed           │
│  ║  │                   │   ║                        │
│  ║  └───────────────────┘   ║                        │
│  ╚═══════════════════════════╝                        │
│                                                        │
│  Next: Wait 0.5s, then yield()...                      │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 11: About to yield()
```
┌────────────────────────────────────────────────────────┐
│           Main Thread About to Yield                   │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Current Execution Point:                              │
│  ┌──────────────────────────────────────────────┐    │
│  │ PUTTOLDC("T1: Prime 2");     ← Done           │    │
│  │ RPI_WaitMicroSeconds(500000); ← Done           │    │
│  │ yield();                      ← ABOUT TO CALL │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  System State BEFORE yield:                            │
│  ┌────────────────────────────────────────────────┐  │
│  │                                                 │  │
│  │  ┌─────────────────────────────────────┐       │  │
│  │  │ current ──▶ &initp (Main Thread)    │       │  │
│  │  │             Status: RUNNING          │       │  │
│  │  │             PC: About to call yield()│       │  │
│  │  └─────────────────────────────────────┘       │  │
│  │                                                 │  │
│  │  ┌─────────────────────────────────────┐       │  │
│  │  │ readyQ ──▶ [T0] ──▶ NULL            │       │  │
│  │  │            Status: WAITING           │       │  │
│  │  │            Function: computePower    │       │  │
│  │  └─────────────────────────────────────┘       │  │
│  │                                                 │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  What will happen:                                     │
│  1. Save main's context                                │
│  2. Move main to back of readyQ                        │
│  3. Load T0's context                                  │
│  4. T0 starts executing                                │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 12: Inside yield() - Part 1
```
┌────────────────────────────────────────────────────────┐
│          yield() Execution - Disable & Check           │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Code Flow:                                            │
│  ┌──────────────────────────────────────────────┐    │
│  │ void yield(void) {                            │    │
│  │     DISABLE();  ← EXECUTED (interrupts off)   │    │
│  │     if (readyQ != NULL) {  ← CHECKING...      │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  CPU State:                                            │
│  ┌──────────────────────────────────────────────┐    │
│  │ ┌────────────────────────────────────┐        │    │
│  │ │ ARM CPU                            │        │    │
│  │ │ ───────────────────────────────    │        │    │
│  │ │ PRIMASK:  1  ← Interrupts BLOCKED  │        │    │
│  │ │ PC:       Inside yield()           │        │    │
│  │ │ SP:       Main's stack             │        │    │
│  │ └────────────────────────────────────┘        │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  Queue Status:                                         │
│  ┌──────────────────────────────────────────────┐    │
│  │ readyQ ──▶ [T0] ──▶ NULL                      │    │
│  │            ↑                                   │    │
│  │            └─ NOT NULL, proceed with switch!   │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  ⚠ CRITICAL SECTION: No interrupts can occur now!     │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 13: Inside yield() - Part 2
```
┌────────────────────────────────────────────────────────┐
│        yield() Execution - Dequeue & Enqueue           │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Code:                                                 │
│  ┌──────────────────────────────────────────────┐    │
│  │ thread p = dequeue(&readyQ); ← EXECUTING      │    │
│  │ enqueue(current, &readyQ);   ← EXECUTING      │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  Step 1: p = dequeue(&readyQ)                          │
│  ┌────────────────────────────────────────────────┐  │
│  │ BEFORE:  readyQ ──▶ [T0] ──▶ NULL             │  │
│  │                                                 │  │
│  │ AFTER:   readyQ ──▶ NULL                       │  │
│  │          p      ──▶ [T0]                       │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  Step 2: enqueue(current, &readyQ)                     │
│  ┌────────────────────────────────────────────────┐  │
│  │ current points to &initp (Main)                 │  │
│  │                                                 │  │
│  │ BEFORE:  readyQ ──▶ NULL                       │  │
│  │                                                 │  │
│  │ AFTER:   readyQ ──▶ [Main] ──▶ NULL           │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  Current State:                                        │
│  ┌────────────────────────────────────────────────┐  │
│  │ p:       Points to Thread 0                     │  │
│  │ current: Still points to Main                   │  │
│  │ readyQ:  [Main] ──▶ NULL                       │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  Next: dispatch(p) will switch to Thread 0             │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 14: dispatch() - Save Main's Context
```
┌────────────────────────────────────────────────────────┐
│       dispatch(p) - Saving Main's Context              │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Code:                                                 │
│  ┌──────────────────────────────────────────────┐    │
│  │ static void dispatch(thread next) {           │    │
│  │     if (current != NULL) {                    │    │
│  │         if (setjmp(current->context) == 0) {  │    │
│  │             // First time: save succeeded     │    │
│  │             current = next;                   │    │
│  │             longjmp(next->context, 1);        │    │
│  │         }                                     │    │
│  │         // Returns here when resumed later    │    │
│  │     }                                         │    │
│  │ }                                             │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  setjmp(current->context) Executes:                    │
│  ┌────────────────────────────────────────────────┐  │
│  │ Saving to: initp.context (Main's jmp_buf)      │  │
│  │                                                 │  │
│  │ Saved Registers:                                │  │
│  │ ┌─────────────┬──────────────────────────┐    │  │
│  │ │ PC (x30)    │ Return to yield() line 194│    │  │
│  │ │ SP (x31)    │ Main's stack pointer      │    │  │
│  │ │ x0-x28      │ Current register values   │    │  │
│  │ │ FP (x29)    │ Frame pointer             │    │  │
│  │ └─────────────┴──────────────────────────┘    │  │
│  │                                                 │  │
│  │ Return Value: 0 (save operation)                │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  Result: Main's execution state preserved!             │
│          Can resume later from this exact point.       │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 15: dispatch() - Restore T0's Context
```
┌────────────────────────────────────────────────────────┐
│      dispatch(p) - Restoring Thread 0's Context        │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Code Continues (setjmp returned 0):                   │
│  ┌──────────────────────────────────────────────┐    │
│  │ current = next;  ← UPDATE: current → Thread 0 │    │
│  │ longjmp(next->context, 1);  ← EXECUTING       │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  longjmp(Thread0->context, 1) Executes:                │
│  ┌────────────────────────────────────────────────┐  │
│  │ Restoring from: threads[0].context              │  │
│  │                                                 │  │
│  │ Restored Registers:                             │  │
│  │ ┌─────────────┬──────────────────────────┐    │  │
│  │ │ PC (x30)    │ spawn() line 172         │    │  │
│  │ │ SP (x31)    │ threads[0].stack[1023]   │    │  │
│  │ │ x0-x28      │ Thread 0's saved values  │    │  │
│  │ │ FP (x29)    │ Thread 0's frame pointer │    │  │
│  │ └─────────────┴──────────────────────────┘    │  │
│  │                                                 │  │
│  │ Special: setjmp() at spawn line 171 will now    │  │
│  │          return 1 (not 0!) due to longjmp arg  │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  ⚡ CONTEXT SWITCH COMPLETE ⚡                          │
│                                                        │
│  System State AFTER:                                   │
│  ┌────────────────────────────────────────────────┐  │
│  │ current: threads[0] (Thread 0)                  │  │
│  │ readyQ:  [Main] ──▶ NULL                       │  │
│  │ CPU PC:  Inside spawn(), line 172               │  │
│  │ CPU SP:  threads[0].stack[1023]                 │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 16: Thread 0 Starts Executing
```
┌────────────────────────────────────────────────────────┐
│          Thread 0 Begins Execution                     │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Location: spawn() function, inside if-block           │
│  ┌──────────────────────────────────────────────┐    │
│  │ if (setjmp(newp->context) == 1) {  ← TRUE!    │    │
│  │     ENABLE();         ← EXECUTING              │    │
│  │     current->function(current->arg);           │    │
│  │     // ... rest of cleanup code                │    │
│  │ }                                             │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  Step 1: ENABLE()                                      │
│  ┌────────────────────────────────────────────────┐  │
│  │ ARM CPU:                                        │  │
│  │ PRIMASK ← 0  (interrupts re-enabled)            │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  Step 2: current->function(current->arg)               │
│  ┌────────────────────────────────────────────────┐  │
│  │ current points to threads[0]                    │  │
│  │ threads[0].function = computePower              │  │
│  │ threads[0].arg = 0                              │  │
│  │                                                 │  │
│  │ Expands to: computePower(0)  ← CALLING!         │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  Now Entering:                                         │
│  ┌──────────────────────────────────────────────┐    │
│  │ void computePower(int seg) {                  │    │
│  │     for(int n = 0; ; n++) {                   │    │
│  │         PUTTOLDC("T%d: %d^2=%d", seg, n, n*n);│    │
│  │         RPI_WaitMicroSeconds(500000);         │    │
│  │         yield();                              │    │
│  │     }                                         │    │
│  │ }                                             │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 17: Thread 0 First Iteration
```
┌────────────────────────────────────────────────────────┐
│       Thread 0: computePower First Output              │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Code Execution:                                       │
│  ┌──────────────────────────────────────────────┐    │
│  │ for(int n = 0; ; n++) {                       │    │
│  │     PUTTOLDC("T%d: %d^2=%d", seg, n, n*n);    │    │
│  │     // seg=0, n=0, n*n=0                      │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  Computation:                                          │
│  ┌────────────────────────────────────────────────┐  │
│  │ seg = 0  (passed as argument)                   │  │
│  │ n   = 0  (first iteration)                      │  │
│  │ n*n = 0  (0 squared)                            │  │
│  │                                                 │  │
│  │ Output: "T0: 0^2=0"                             │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  ╔═══════════════════════════╗                        │
│  ║  PiFace LCD Display       ║                        │
│  ║  ┌───────────────────┐   ║                        │
│  ║  │ T0: 0^2=0         │   ║  ← NEW DISPLAY         │
│  ║  │                   │   ║     (replaced T1: Prime │
│  ║  └───────────────────┘   ║      2)                │
│  ╚═══════════════════════════╝                        │
│                                                        │
│  Next:                                                 │
│  ┌──────────────────────────────────────────────┐    │
│  │ RPI_WaitMicroSeconds(500000);  ← Wait 0.5s    │    │
│  │ yield();                       ← Then yield   │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

---

## Animation 3: Round-Robin Steady State

### Frame 18: Continuous Alternation
```
┌────────────────────────────────────────────────────────┐
│           Round-Robin Pattern Established              │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Cycle Repeats Infinitely:                             │
│                                                        │
│  ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓    │
│  ┃ ITERATION 1                                    ┃    │
│  ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫    │
│  ┃ Main:     "T1: Prime 2"     [0.5s]  yield()   ┃    │
│  ┃ Thread 0: "T0: 0^2=0"       [0.5s]  yield()   ┃    │
│  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛    │
│                                                        │
│  ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓    │
│  ┃ ITERATION 2                                    ┃    │
│  ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫    │
│  ┃ Main:     "T1: Prime 3"     [0.5s]  yield()   ┃    │
│  ┃ Thread 0: "T0: 1^2=1"       [0.5s]  yield()   ┃    │
│  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛    │
│                                                        │
│  ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓    │
│  ┃ ITERATION 3                                    ┃    │
│  ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫    │
│  ┃ Main:     "T1: Prime 5"     [0.5s]  yield()   ┃    │
│  ┃ Thread 0: "T0: 2^2=4"       [0.5s]  yield()   ┃    │
│  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛    │
│                                                        │
│  Queue State (continuously rotating):                  │
│  ┌────────────────────────────────────────────────┐  │
│  │ When Main running:                              │  │
│  │   current: Main                                 │  │
│  │   readyQ:  [T0] ──▶ NULL                       │  │
│  │                                                 │  │
│  │ When Thread 0 running:                          │  │
│  │   current: Thread 0                             │  │
│  │   readyQ:  [Main] ──▶ NULL                     │  │
│  └────────────────────────────────────────────────┘  │
│                                                        │
│  Perfect Fairness:                                     │
│  Each thread gets equal CPU time (50% each)            │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Frame 19: Timeline Visualization (First 5 Seconds)
```
┌────────────────────────────────────────────────────────┐
│              Execution Timeline (0-5 seconds)          │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Time (seconds)                                        │
│  0         1         2         3         4         5   │
│  ├─────────┼─────────┼─────────┼─────────┼─────────┤ │
│  │                                                    │ │
│  Main Thread:                                          │
│  ░░░░░░░░░░                    ░░░░░░░░░░            │ │
│  │ Prime 2 │    (waiting)      │ Prime 3 │            │ │
│  0.0     0.5                  1.0      1.5            │ │
│                 ░░░░░░░░░░                             │ │
│                 │ Prime 5 │     (waiting)              │ │
│                2.0      2.5                            │ │
│                                                        │ │
│  Thread 0:                                             │ │
│            ▓▓▓▓▓▓▓▓▓▓                    ▓▓▓▓▓▓▓▓▓▓   │ │
│            │  0^2=0  │    (waiting)      │  1^2=1  │   │ │
│           0.5       1.0                 1.5       2.0   │ │
│                                ▓▓▓▓▓▓▓▓▓▓               │ │
│                                │  2^2=4  │               │ │
│                               2.5       3.0             │ │
│                                                        │ │
│  Legend:                                               │ │
│  ░░░ = Main thread executing                           │ │
│  ▓▓▓ = Thread 0 executing                              │ │
│  (space) = Thread waiting in readyQ                    │ │
│                                                        │ │
│  Note: Each thread runs for exactly 0.5 seconds,       │ │
│        then yields to the other thread.                │ │
│                                                        │ │
└────────────────────────────────────────────────────────┘
```

---

## Animation 4: Queue State Transitions

### Frame 20-25: One Complete Cycle (6 Snapshots)
```
┌────────────────────────────────────────────────────────┐
│          Snapshot 1: Main Running                      │
├────────────────────────────────────────────────────────┤
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │ CURRENT (Running):                            │    │
│  │ ┌──────────────────────────────────────┐     │    │
│  │ │  Main Thread                          │     │    │
│  │ │  Executing: computePrimes(1)          │     │    │
│  │ │  Computing is_prime(n)...             │     │    │
│  │ └──────────────────────────────────────┘     │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │ READY QUEUE (Waiting):                        │    │
│  │ ┌──────────────────────────────────────┐     │    │
│  │ │  Thread 0                             │     │    │
│  │ │  Function: computePower(0)            │     │    │
│  │ │  Status: Suspended, waiting for CPU   │     │    │
│  │ └──────────────────────────────────────┘     │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────┐
│       Snapshot 2: Main Calls yield()                   │
├────────────────────────────────────────────────────────┤
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │ CURRENT (About to switch):                    │    │
│  │ ┌──────────────────────────────────────┐     │    │
│  │ │  Main Thread                          │     │    │
│  │ │  Inside yield() function              │     │    │
│  │ │  Interrupts: DISABLED                 │     │    │
│  │ └──────────────────────────────────────┘     │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │ READY QUEUE:                                  │    │
│  │ ┌──────────────────────────────────────┐     │    │
│  │ │  Thread 0  ← About to be dequeued     │     │    │
│  │ └──────────────────────────────────────┘     │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────┐
│      Snapshot 3: Mid-Switch (setjmp/longjmp)           │
├────────────────────────────────────────────────────────┤
│                                                        │
│  ⚡ CONTEXT SWITCH IN PROGRESS ⚡                       │
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │ Saving Main's context...          ✓ Done     │    │
│  │ Moving Main to readyQ...           ✓ Done     │    │
│  │ Loading Thread 0's context...      ✓ Done     │    │
│  │ Jumping to Thread 0...             ⚡ NOW      │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │ READY QUEUE:                                  │    │
│  │ ┌──────────────────────────────────────┐     │    │
│  │ │  Main Thread  ← Just enqueued         │     │    │
│  │ └──────────────────────────────────────┘     │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────┐
│         Snapshot 4: Thread 0 Running                   │
├────────────────────────────────────────────────────────┤
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │ CURRENT (Running):                            │    │
│  │ ┌──────────────────────────────────────┐     │    │
│  │ │  Thread 0                             │     │    │
│  │ │  Executing: computePower(0)           │     │    │
│  │ │  Computing n^2...                     │     │    │
│  │ └──────────────────────────────────────┘     │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │ READY QUEUE (Waiting):                        │    │
│  │ ┌──────────────────────────────────────┐     │    │
│  │ │  Main Thread                          │     │    │
│  │ │  Function: computePrimes(1)           │     │    │
│  │ │  Status: Suspended at yield()         │     │    │
│  │ └──────────────────────────────────────┘     │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────┐
│       Snapshot 5: Thread 0 Calls yield()               │
├────────────────────────────────────────────────────────┤
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │ CURRENT (About to switch):                    │    │
│  │ ┌──────────────────────────────────────┐     │    │
│  │ │  Thread 0                             │     │    │
│  │ │  Inside yield() function              │     │    │
│  │ │  Interrupts: DISABLED                 │     │    │
│  │ └──────────────────────────────────────┘     │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │ READY QUEUE:                                  │    │
│  │ ┌──────────────────────────────────────┐     │    │
│  │ │  Main Thread ← About to be dequeued   │     │    │
│  │ └──────────────────────────────────────┘     │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────┐
│      Snapshot 6: Back to Main (Cycle Complete)         │
├────────────────────────────────────────────────────────┤
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │ CURRENT (Running):                            │    │
│  │ ┌──────────────────────────────────────┐     │    │
│  │ │  Main Thread                          │     │    │
│  │ │  Resumed after yield()                │     │    │
│  │ │  Incrementing n, checking is_prime()  │     │    │
│  │ └──────────────────────────────────────┘     │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │ READY QUEUE (Waiting):                        │    │
│  │ ┌──────────────────────────────────────┐     │    │
│  │ │  Thread 0                             │     │    │
│  │ │  Status: Suspended at yield()         │     │    │
│  │ └──────────────────────────────────────┘     │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  ↻ CYCLE REPEATS FROM SNAPSHOT 1                      │
│                                                        │
└────────────────────────────────────────────────────────┘
```

---

## Summary: Key Moments

### Critical Events in Order:

1. **System Boot** → Initialize hardware
2. **initializeThreads()** → Setup thread pool and freeQ
3. **spawn(computePower, 0)** → Create Thread 0
   - Allocate from freeQ
   - Configure function and argument
   - Setup context with setjmp
   - Modify stack pointer
   - Enqueue to readyQ
4. **computePrimes(1)** → Main thread starts
5. **First yield()** → Trigger context switch
   - Save Main's context
   - Dequeue Thread 0
   - Enqueue Main
   - Restore Thread 0's context
   - Jump to Thread 0
6. **Thread 0 Executes** → Display "T0: 0^2=0"
7. **Thread 0 yields()** → Switch back to Main
8. **Round-Robin Loop** → Infinite alternation

### Cooperative Nature

```
┌────────────────────────────────────────────────────────┐
│        Why "Cooperative" Multithreading?               │
├────────────────────────────────────────────────────────┤
│                                                        │
│  ❌ NO forced preemption                               │
│     - Timer interrupts don't cause context switches    │
│     - No scheduler interrupt handler                   │
│                                                        │
│  ✅ Threads voluntarily yield                          │
│     - Must explicitly call yield()                     │
│     - Threads "cooperate" by giving up CPU             │
│                                                        │
│  Consequence:                                          │
│  ┌──────────────────────────────────────────────┐    │
│  │ If a thread never calls yield():              │    │
│  │   → It runs forever                           │    │
│  │   → Other threads starve                      │    │
│  │   → System becomes unresponsive               │    │
│  │                                               │    │
│  │ Example:                                      │    │
│  │   while(1);  // BUG! No yield() = hangs       │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
│  Why use cooperative scheduling?                       │
│  + Simple implementation                               │
│  + Predictable execution (no surprise switches)        │
│  + Low overhead (no timer interrupts)                  │
│  + Easy to debug                                       │
│                                                        │
│  - Requires disciplined programming                    │
│  - One bad thread ruins everything                     │
│  - Not suitable for untrusted code                     │
│                                                        │
└────────────────────────────────────────────────────────┘
```

---

## Conclusion

This animated walkthrough demonstrates:

1. **Initialization**: How the thread pool is setup
2. **Spawning**: Creating new threads with proper context
3. **Context Switching**: The magic of setjmp/longjmp
4. **Round-Robin**: Fair scheduling through yield()
5. **Queue Management**: Thread lifecycle through queues

The key insight: **Cooperative multithreading is simple but requires programmer discipline.** Every thread must regularly call `yield()` or the entire system blocks.

**Next Steps (Assignment 4)**:
- Add mutex support for shared resource protection
- Implement priority scheduling (RM/EDF)
- Add preemptive scheduling with timer interrupts
- Support periodic task respawning

---

**End of Animated Walkthrough**

*For best understanding, read this alongside the main report and diagrams.*

