# Detailed Flow: Scheduler, Respawn, and Sort Logic

## Overview

This document provides a comprehensive flow diagram showing how the scheduler, respawn_periodic_tasks, and sort functions work together in Assignment 4 Part 2 (Rate Monotonic Scheduling) and Part 3 (Earliest Deadline First Scheduling).

**Important**: This document describes both the theoretical/ideal implementation AND the actual implementation found in a4p2.c, highlighting differences where they exist.

---

## System Architecture

### Queues

The system maintains three main queues:
1. **freeQ**: Available thread blocks (pool of unused threads)
2. **readyQ**: Ready to execute threads (sorted by priority)
3. **doneQ**: Completed threads awaiting respawn

### Thread Structure

```c
struct thread_block {
    short idx;                              // Unique identifier
    void (*function)(int);                  // Code to run
    int arg;                                // Argument to function
    thread next;                            // For linked lists
    jmp_buf context;                        // Machine state
    char stack[STACKSIZE];                  // Execution stack space
    unsigned int Period_Deadline;          // Absolute Period and Deadline
    unsigned int Rel_Period_Deadline;      // Relative Period and Deadline
};
```

### Global Variables

```c
thread freeQ = threads;                     // Points to available threads
thread readyQ = NULL;                       // Ready queue (empty initially)
thread doneQ = NULL;                        // Done queue (empty initially)
thread current = &initp;                    // Currently executing thread
```

---

## Visual Flowcharts

### High-Level Scheduler Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                    TIMER INTERRUPT FIRES                        │
│                     (every tick)                                │
└──────────────────────────────┬──────────────────────────────────┘
                               │
                               ▼
                    ┌──────────────────────┐
                    │  interrupt_vector()  │
                    │  - Clear interrupt   │
                    │  - ticks++           │
                    │  - call scheduler()  │
                    └──────────┬───────────┘
                               │
                               ▼
                    ┌──────────────────────┐
                    │   scheduler()        │
                    │                      │
                    │   1. respawn_        │
                    │      periodic_tasks()│
                    │                      │
                    │   2. scheduler_RM()  │
                    │      or              │
                    │      scheduler_EDF() │
                    └──────────┬───────────┘
                               │
                               ▼
                    ┌──────────────────────┐
                    │   dispatch(next)     │
                    │  - Save current      │
                    │  - Jump to next      │
                    └──────────────────────┘
```

### Queue Transitions Flow

```
┌─────────────────────────────────────────────────────────────┐
│                        freeQ                                │
│              (Available Thread Pool)                        │
│                                                             │
│   T[0] → T[1] → T[2] → T[3] → T[4] → NULL                │
└────────────────────┬────────────────────────────────────────┘
                     │
                     │ spawnWithDeadline()
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                        readyQ                               │
│         (Sorted by Priority - Ready to Execute)             │
│                                                             │
│   T[1] (P=3) → T[0] (P=5) → T[2] (P=7) → NULL            │
│   Highest Priority    ↑                          Lowest    │
└────────────────────┬────────────────────────────────────────┘
                     │
                     │ scheduler_RM() / scheduler_EDF()
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                    Current (Executing)                      │
│                                                             │
│                        T[1]                                 │
└────────────────────┬────────────────────────────────────────┘
                     │
                     │ Task completes
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                        doneQ                                │
│              (Completed - Awaiting Respawn)                 │
│                                                             │
│   T[1] → NULL                                               │
└────────────────────┬────────────────────────────────────────┘
                     │
                     │ respawn_periodic_tasks()
                     │ (when period elapsed)
                     ▼
               ┌──────────────────────┐
               │    Back to readyQ    │
               │  (Cycle continues)   │
               └──────────────────────┘
```

---

## Part 1: System Initialization

### Initial State

```
┌─────────────────────────────────────────────────────────────────┐
│                         INITIAL STATE                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  freeQ:  T[0] → T[1] → T[2] → T[3] → T[4] → NULL             │
│          [Available thread pool]                                │
│                                                                 │
│  readyQ: NULL                                                   │
│          [No ready threads]                                     │
│                                                                 │
│  doneQ:  NULL                                                   │
│          [No completed threads]                                 │
│                                                                 │
│  current: &initp (main thread, never scheduled)                 │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Part 2: Spawn With Deadline

### Function: `spawnWithDeadline(function, arg, deadline, rel_deadline)`

**Purpose**: Create periodic tasks with assigned periods and deadlines

**Example**:
```c
spawnWithDeadline(computeSomething, 0, 5, 5);  // Period=5, Deadline=5
spawnWithDeadline(computeSomething, 1, 3, 3);  // Period=3, Deadline=3
spawnWithDeadline(computeSomething, 2, 7, 7);  // Period=7, Deadline=7
```

**Flow**:

```
1. DISABLE interrupts
   ↓
2. Check if initialized (if not, initialize threads)
   ↓
3. newp = dequeue(&freeQ)           // Take thread from free pool
   ↓
4. Set thread attributes:
   - newp->function = function
   - newp->arg = arg
   - newp->Period_Deadline = deadline
   - newp->Rel_Period_Deadline = rel_deadline
   ↓
5. Save context (setjmp) for later execution
   ↓
6. enqueue(newp, &readyQ)          // Add to ready queue
   ↓
7. ENABLE interrupts
   ↓
8. Sort readyQ by Period_Deadline  // RM: shorter period = higher priority
                                     // EDF: earlier deadline = higher priority
```

**Queue Transformation**:

```
BEFORE spawnWithDeadline(computeSomething, 1, 3, 3):
┌─────────────────────────────────────────────────────────────────┐
│  freeQ:  T[0] → T[1] → T[2] → T[3] → T[4] → NULL             │
│  readyQ: NULL                                                   │
│  doneQ:  NULL                                                   │
└─────────────────────────────────────────────────────────────────┘

AFTER spawnWithDeadline(computeSomething, 1, 3, 3):
┌─────────────────────────────────────────────────────────────────┐
│  freeQ:  T[0] → T[2] → T[3] → T[4] → NULL                     │
│  readyQ: T[1] → NULL  (Period_Deadline=3)                       │
│  doneQ:  NULL                                                   │
└─────────────────────────────────────────────────────────────────┘
```

---

## Part 3: Scheduler Main Flow

### Function: `scheduler()`

**Called by**: Timer interrupt handler (`interrupt_vector()`)

**Flow**:

```
Timer Interrupt Fires (every tick, e.g., 0xF3C cycles)
   ↓
interrupt_vector() called
   ↓
ticks++                                       // Increment global counter
   ↓
scheduler() called
   ↓
respawn_periodic_tasks()                      // FIRST: Respawn periodic tasks
   ↓
scheduler_RM() or scheduler_EDF()             // THEN: Select next task
   ↓
Context switch to selected task
```

---

## Part 4: Respawn Periodic Tasks

### Function: `respawn_periodic_tasks()`

**Purpose**: Move completed periodic tasks from doneQ back to readyQ after their period

**⚠️ Critical Issue in Actual Implementation**: When a task first completes and is enqueued to doneQ (line 258 in spawnWithDeadline), the code does NOT set the next deadline. This means:

1. First time a task completes: `Period_Deadline` still has the initial value from spawn (e.g., 3, 5, or 7)
2. `respawn_periodic_tasks()` checks `ticks >= Period_Deadline`, which would be true immediately for the first period
3. This assumes the task should already have its next deadline set

**Proper Fix**: Before enqueuing to doneQ, update the deadline:
```c
current->Period_Deadline = ticks + current->Rel_Period_Deadline;
enqueue(current, &doneQ);
```

**Detailed Flowchart**:

```
┌─────────────────────────────────────────────────────────────────┐
│              respawn_periodic_tasks()                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  thread t = doneQ;              // Start at head of doneQ      │
│  thread prev = NULL;             // Previous node              │
│                                                                 │
│  while (t != NULL) {            // Iterate through doneQ       │
│                                                                 │
│      ├─ Check: ticks % t->Rel_Period_Deadline == 0?           │
│      │                                                          │
│      │   YES ──→  Period elapsed!                              │
│      │            ┌──────────────────────┐                     │
│      │            │ 1. Remove from doneQ │                     │
│      │            │    (unlink node)     │                     │
│      │            │                      │                     │
│      │            │ 2. Reset deadline:   │                     │
│      │            │    t->Period_        │                     │
│      │            │    Deadline =        │                     │
│      │            │    t->Rel_Period_    │                     │
│      │            │    Deadline          │                     │
│      │            │                      │                     │
│      │            │ 3. Add to readyQ     │                     │
│      │            │    enqueue(t, &readyQ)                     │
│      │            │                      │                     │
│      │            │ 4. Sort readyQ       │                     │
│      │            │    sort(&readyQ)     │                     │
│      │            │                      │                     │
│      │            │ 5. Move to next     │                     │
│      │            │    t = t->next       │                     │
│      │            └──────────────────────┘                     │
│      │                                                          │
│      │   NO  ──→  Period not elapsed yet                       │
│      │            ┌──────────────────────┐                     │
│      │            │ Move to next task:   │                     │
│      │            │   prev = t;          │                     │
│      │            │   t = t->next;       │                     │
│      │            └──────────────────────┘                     │
│      │                                                          │
│  }                                                              │
│                                                                 │
│  DONE: All tasks checked                                        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

**Logic**:

```
Note: The actual implementation uses a different approach than described in earlier documentation.

For each task in doneQ:
    Check if (ticks >= task->Period_Deadline)
        ↓
    If YES: Task's period has elapsed
        ↓
        Dequeue task from doneQ
        ↓
        Update task's Period_Deadline += Rel_Period_Deadline (for next period)
        ↓
        Reset task's context stack
        ↓
        Enqueue task to readyQ
        ↓
    If NO: Keep in doneQ for later
        ↓
        Move to tempDoneQ
```

**Important Difference**: The implementation uses absolute time comparison (`ticks >= Period_Deadline`) rather than modular arithmetic (`ticks % Rel_Period_Deadline == 0`), and it increments the deadline rather than resetting it.

**Example Flow** (Based on Actual Implementation):

**Initial State** (after all tasks spawned with a4p2.c):
```
┌─────────────────────────────────────────────────────────────────┐
│  freeQ:  T[3] → T[4] → NULL                                   │
│                                                                 │
│  readyQ: T[0] → T[1] → T[2] → NULL                             │
│          [Not initially sorted - needs sort after spawn]       │
│          Task 0: deadline=5, rel_period=5                       │
│          Task 1: deadline=3, rel_period=3                       │
│          Task 2: deadline=7, rel_period=7                       │
│                                                                 │
│  doneQ:  NULL                                                   │
│  current: initp (main thread)                                   │
│  ticks: 0                                                       │
└─────────────────────────────────────────────────────────────────┘

NOTE: In a4p2.c, spawnWithDeadline doesn't sort automatically.
      Tasks are sorted by scheduler_RM() before dispatching.
```

**Tick 0 → Tick 1**: First context switch
```
┌─────────────────────────────────────────────────────────────────┐
│  Timer interrupt fires                                          │
│  SCHEDULER CALLED                                               │
│                                                                 │
│  scheduler_RM():                                                │
│    → respawn_periodic_tasks(): doneQ empty, nothing to do      │
│    → sort(&readyQ): sorts by Rel_Period_Deadline ascending     │
│       Result: T[1] (3) → T[0] (5) → T[2] (7) → NULL           │
│    → yield(): switches from initp to T[1]                      │
│                                                                 │
│  Task T[1] starts executing (computeSomething(1))              │
│  T[1] captures ticks=1, computes exp(10), displays "S1: 1"     │
│  T[1] waits until ticks advances (busy-wait loop)              │
│                                                                 │
│  ticks: 1                                                       │
│  current: T[1]                                                  │
│  readyQ: T[0] → T[2] → NULL                                    │
└─────────────────────────────────────────────────────────────────┘
```

**Tick 1 → Tick 2**: Task completes, moves to doneQ
```
┌─────────────────────────────────────────────────────────────────┐
│  ticks changes from 1 to 2                                      │
│  T[1] busy-wait loop exits (t != ticks now)                    │
│  T[1] function returns                                          │
│                                                                 │
│  Back in spawnWithDeadline's return handler:                   │
│    → enqueue(current, &doneQ) // T[1] goes to doneQ           │
│    → T[1]->Period_Deadline = 3 (next deadline at tick 6)      │
│       (Implementation sets: ticks + Rel_Period_Deadline)       │
│    → dequeue(&readyQ) // Get next task T[0]                   │
│    → dispatch(T[0]) // Switch to T[0]                         │
│                                                                 │
│  doneQ: T[1] (Period_Deadline=6) → NULL                       │
│  current: T[0]                                                  │
│  readyQ: T[2] → NULL                                           │
│  ticks: 2                                                       │
└─────────────────────────────────────────────────────────────────┘
```

**Tick 2 → Tick 6**: Respawn T[1]
```
┌─────────────────────────────────────────────────────────────────┐
│  Tick 3, 4, 5: T[0] or T[2] running                            │
│                                                                 │
│  Tick 6: Timer interrupt fires                                 │
│  SCHEDULER CALLED                                               │
│                                                                 │
│  scheduler_RM():                                                │
│    → respawn_periodic_tasks():                                 │
│       - Check T[1]: ticks=6, T[1]->Period_Deadline=6           │
│       - 6 >= 6 ✓ (period elapsed!)                             │
│       - Dequeue T[1] from doneQ                                │
│       - T[1]->Period_Deadline += 3 → now 9                     │
│       - Reset T[1] context with setjmp/SETSTACK                │
│       - Enqueue T[1] to readyQ                                 │
│       - doneQ: NULL                                             │
│    → sort(&readyQ): sorts readyQ by period                     │
│       readyQ now: T[1] (3) → T[2] (7) → NULL (if T[0] done)   │
│    → yield(): may preempt current if T[1] higher priority      │
│                                                                 │
│  ticks: 6                                                       │
│  doneQ: NULL                                                    │
│  readyQ: T[1] → T[2] → NULL                                    │
└─────────────────────────────────────────────────────────────────┘
```

---

## Part 5: Sort Logic

### Function: `sort(thread *queue)`

**Purpose**: Maintain readyQ sorted by priority

**Detailed Flowchart (Insertion Sort)**:

```
┌─────────────────────────────────────────────────────────────────┐
│                    sort(thread *queue)                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  if (*queue == NULL || (*queue)->next == NULL)                 │
│      return;  // Empty or single element, already sorted       │
│                                                                 │
│  // Insertion sort algorithm                                   │
│  thread sorted = NULL;        // Start with empty sorted list │
│  thread current = *queue;     // Process unsorted list        │
│                                                                 │
│  while (current != NULL) {                                      │
│                                                                 │
│      thread next = current->next;   // Save next               │
│      thread key = current;           // Current task to insert │
│                                                                 │
│      // Find correct position in sorted list                  │
│      thread *sorted_ptr = &sorted;                            │
│                                                                 │
│      while (*sorted_ptr != NULL &&                            │
│             (*sorted_ptr)->Period_Deadline <=                 │
│             key->Period_Deadline) {                            │
│          sorted_ptr = &((*sorted_ptr)->next);                 │
│      }                                                          │
│                                                                 │
│      // Insert key before *sorted_ptr                         │
│      key->next = *sorted_ptr;                                 │
│      *sorted_ptr = key;                                       │
│                                                                 │
│      current = next;  // Move to next unsorted task           │
│  }                                                              │
│                                                                 │
│  *queue = sorted;  // Update queue head                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

**Visual Example**:

```
Step 1: readyQ = T[0] (P=5) → T[1] (P=3) → T[2] (P=7) → NULL
                             ↑ out of order

Step 2: Insert T[0] (P=5)
        sorted = T[0] (P=5) → NULL

Step 3: Insert T[1] (P=3)
        Compare: 3 < 5 → Insert at head
        sorted = T[1] (P=3) → T[0] (P=5) → NULL

Step 4: Insert T[2] (P=7)
        Compare: 7 > 3, 7 > 5 → Insert at tail
        sorted = T[1] (P=3) → T[0] (P=5) → T[2] (P=7) → NULL

Result: readyQ sorted by period (ascending)
```

**Sorting Criteria**:
- **RM (Rate Monotonic)**: Ascending by `Period_Deadline` (shorter period = higher priority)
- **EDF (Earliest Deadline First)**: Ascending by `Period_Deadline` (earlier deadline = higher priority)

**Implementation Options**:

#### Option 1: Insertion Sort (Recommended)

```
Algorithm: Insertion Sort
For each task in unsorted portion of readyQ:
    - Find correct position (based on Period_Deadline)
    - Insert task at that position
    - Maintain queue structure
```

**Example**:
```
Before: readyQ = T[0] (P=5) → T[1] (P=3) → NULL
                              ↑ out of order

Step 1: Find position for T[1] (P=3)
        T[1] should come before T[0] (3 < 5)

Step 2: Re-link:
        readyQ = T[1] → NULL
        readyQ = T[1] → T[0] → NULL

After:  readyQ = T[1] (P=3) → T[0] (P=5) → NULL
```

#### Option 2: Bubble Sort (Simpler, slower)

```
Algorithm: Bubble Sort
For N iterations:
    - Compare adjacent tasks
    - Swap if out of order
    - Continue until no swaps needed
```

#### Option 3: Merge Sort (Most efficient for large queues)

```
Algorithm: Merge Sort
- Recursively divide queue into halves
- Sort each half
- Merge sorted halves
```

**When to Call Sort**:
1. After respawn_periodic_tasks() adds tasks to readyQ
2. After spawnWithDeadline() adds new task
3. Before scheduler_RM() or scheduler_EDF() selects next task

---

## Part 6: Scheduler RM (Rate Monotonic)

### Function: `scheduler_RM()`

**Priority Rule**: Shorter period = higher priority

**Detailed Flowchart**:

```
┌─────────────────────────────────────────────────────────────────┐
│                    scheduler_RM()                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  if (readyQ == NULL) {                                         │
│      return;  // No ready tasks, keep executing current        │
│  }                                                              │
│                                                                 │
│  // Get highest priority task (head of sorted readyQ)         │
│  thread next = readyQ;                                         │
│                                                                 │
│  if (current == &initp) {                                      │
│      // Main thread, always switch to real task               │
│      dispatch(next);                                           │
│      return;                                                   │
│  }                                                              │
│                                                                 │
│  // Check if current task should continue or preempt          │
│  if (current->Period_Deadline < next->Period_Deadline) {     │
│      // Current has higher priority, continue execution       │
│      return;                                                   │
│  } else {                                                      │
│      // Preempt: switch to next task                          │
│      enqueue(current, &readyQ);  // Put current back         │
│      readyQ = readyQ->next;     // Remove next from queue    │
│      sort(&readyQ);              // Re-sort if needed        │
│      dispatch(next);            // Switch to next            │
│  }                                                              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

**Flow**:

```
1. Check if readyQ is not empty
   ↓
2. If empty, do nothing (idle)
   ↓
3. If not empty:
   a. The head of readyQ has highest priority (shortest period)
   b. If current task should preempt:
      - Enqueue current to readyQ (if not done)
      - Dequeue next from readyQ
      - dispatch(next)
   c. If current task is not done:
      - Continue execution
```

**Example**:

```
State:
┌─────────────────────────────────────────────────────────────────┐
│  readyQ: T[1] (P=3) → T[0] (P=5) → T[2] (P=7) → NULL          │
│                                                                 │
│  current: T[1] (executing)                                      │
│  ticks: 1                                                       │
└─────────────────────────────────────────────────────────────────┘

Timer interrupt fires:
   ↓
scheduler_RM():
   - Current task T[1] is still highest priority (P=3)
   - Allow T[1] to continue
   - No context switch
```

```
State:
┌─────────────────────────────────────────────────────────────────┐
│  readyQ: T[0] (P=5) → T[2] (P=7) → NULL                        │
│                                                                 │
│  doneQ: T[1] (completed)                                        │
│                                                                 │
│  current: NULL (or next task)                                   │
│  ticks: 2                                                       │
└─────────────────────────────────────────────────────────────────┘

Timer interrupt fires:
   ↓
respawn_periodic_tasks():
   - Check T[1]: 2 % 3 != 0 (not time to respawn yet)
   
scheduler_RM():
   - Dequeue T[0] (P=5) from readyQ
   - Set current = T[0]
   - dispatch(T[0])
```

---

## Part 7: Scheduler EDF (Earliest Deadline First)

### Function: `scheduler_EDF()`

**Priority Rule**: Earlier deadline = higher priority

**Key Difference from RM**: Deadlines are **dynamically updated** at each tick

**Detailed Flowchart**:

```
┌─────────────────────────────────────────────────────────────────┐
│                    scheduler_EDF()                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  if (readyQ == NULL) {                                         │
│      return;  // No ready tasks                                 │
│  }                                                              │
│                                                                 │
│  // CRITICAL: Update all task deadlines                        │
│  thread t = readyQ;                                            │
│  while (t != NULL) {                                           │
│      if (t->Period_Deadline > 0) {                             │
│          t->Period_Deadline = t->Period_Deadline - 1;         │
│      }                                                          │
│      t = t->next;                                              │
│  }                                                              │
│                                                                 │
│  // Sort readyQ by updated deadlines                           │
│  sort(&readyQ);                                                 │
│                                                                 │
│  // Select highest priority (earliest deadline)                │
│  thread next = readyQ;                                         │
│                                                                 │
│  if (current == &initp) {                                      │
│      // Main thread, switch to real task                      │
│      dispatch(next);                                           │
│      return;                                                   │
│  }                                                              │
│                                                                 │
│  // Check if preemption needed                                │
│  if (current->Period_Deadline <= next->Period_Deadline) {    │
│      // Current has earlier or equal deadline                 │
│      return;  // Continue current task                        │
│  } else {                                                      │
│      // Preempt: higher priority task available               │
│      enqueue(current, &readyQ);                               │
│      readyQ = readyQ->next;                                   │
│      sort(&readyQ);                                           │
│      dispatch(next);                                          │
│  }                                                              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

**Flow**:

```
1. Update all readyQ task deadlines:
   For each task in readyQ:
       task->Period_Deadline = task->Period_Deadline - 1
   ↓
2. Sort readyQ by Period_Deadline (ascending)
   ↓
3. Select highest priority task (head of readyQ)
   ↓
4. Context switch if needed
```

**Example**:

```
Initial State (tick 0):
┌─────────────────────────────────────────────────────────────────┐
│  readyQ: T[0] (D=5) → T[1] (D=3) → T[2] (D=7) → NULL          │
│                                                                 │
│  current: T[1] (executing)                                      │
└─────────────────────────────────────────────────────────────────┘

Tick 1:
scheduler_EDF():
   1. Update deadlines:
      T[0]: D=5 → D=4
      T[1]: D=3 → D=2
      T[2]: D=7 → D=6
    
   2. Sort readyQ:
      T[1] (D=2) → T[0] (D=4) → T[2] (D=6) → NULL
    
   3. Highest priority: T[1] (D=2)
      Continue T[1] execution
```

```
Tick 2 (after T[1] completes):
┌─────────────────────────────────────────────────────────────────┐
│  readyQ: T[0] (D=4) → T[2] (D=6) → NULL                        │
│                                                                 │
│  doneQ: T[1] (completed)                                        │
│                                                                 │
│  ticks: 2                                                       │
└─────────────────────────────────────────────────────────────────┘

Tick 3:
scheduler_EDF():
   1. Update deadlines:
      T[0]: D=4 → D=3
      T[2]: D=6 → D=5
    
   2. respawn_periodic_tasks():
      - Check T[1]: 3 % 3 == 0 ✓
      - T[1] respawned with D=3
      - readyQ: T[0] (D=3) → T[1] (D=3) → T[2] (D=5) → NULL
      - Sort by deadline: [T[0], T[1] both D=3] → T[2] (D=5)
    
   3. Highest priority: T[0] or T[1] (both D=3)
      Select first one
```

---

## Part 8: Complete Example Flow

### Complete System Execution (RM Scheduler)

**Setup**:
```c
spawnWithDeadline(computeSomething, 0, 5, 5);  // Task A: Period=5
spawnWithDeadline(computeSomething, 1, 3, 3);  // Task B: Period=3 (highest priority)
spawnWithDeadline(computeSomething, 2, 7, 7);  // Task C: Period=7
initTimerInterrupts();
```

**Timeline**:

```
┌──────┬──────────┬──────────┬──────────┬──────────┬────────────┐
│ Tick │ Current  │  readyQ  │  doneQ   │   sort   │  respawn   │
├──────┼──────────┼──────────┼──────────┼──────────┼────────────┤
│  0   │  initp   │ B→A→C    │ NULL     │   Yes    │   None     │
│      │          │ (3,5,7)  │          │          │            │
├──────┼──────────┼──────────┼──────────┼──────────┼────────────┤
│  1   │    B     │ A→C      │   B      │   No     │   None     │
│      │ (exec)   │ (5,7)    │          │          │            │
├──────┼──────────┼──────────┼──────────┼──────────┼────────────┤
│  2   │    A     │ C        │   B      │   No     │   None     │
│      │ (exec)   │ (7)      │          │          │            │
├──────┼──────────┼──────────┼──────────┼──────────┼────────────┤
│  3   │    C     │ B        │ B,A      │   No     │  B→readyQ  │
│      │ (exec)   │ (3)      │          │   Yes    │   (3%3=0)  │
├──────┼──────────┼──────────┼──────────┼──────────┼────────────┤
│  4   │    B     │ NULL     │ B,A      │   No     │   None     │
│      │ (exec)   │          │          │          │            │
├──────┼──────────┼──────────┼──────────┼──────────┼────────────┤
│  5   │  idle    │ B,A      │ B,A      │   Yes    │  B→readyQ  │
│      │          │ (3,5)    │          │          │  A→readyQ  │
│      │          │          │          │          │   (5%5=0)  │
├──────┼──────────┼──────────┼──────────┼──────────┼────────────┤
│  6   │    B     │ A        │ B,A      │   No     │   None     │
│      │ (exec)   │ (5)      │          │          │            │
├──────┼──────────┼──────────┼──────────┼──────────┼────────────┤
│  7   │    A     │ B        │ B,A      │   No     │   None     │
│      │ (exec)   │ (3)      │          │          │            │
├──────┼──────────┼──────────┼──────────┼──────────┼────────────┤
│  8   │    B     │ C        │ B,A,C    │   No     │   None     │
│      │ (exec)   │ (7)      │          │          │            │
├──────┼──────────┼──────────┼──────────┼──────────┼────────────┤
│  9   │    C     │ B        │ B,A,C    │   No     │  B→readyQ  │
│      │ (exec)   │ (3)      │          │   Yes    │   (9%3=3)  │
│      │          │          │          │          │   wait...  │
├──────┼──────────┼──────────┼──────────┼──────────┼────────────┤
│ 10   │    B     │ A,C      │ B,A,C    │   Yes    │  None      │
│      │ (exec)   │ (5,7)    │          │          │            │
└──────┴──────────┴──────────┴──────────┴──────────┴────────────┘
```

**Pattern**: Task B (period=3) runs every 3 ticks, Task A (period=5) runs every 5 ticks, Task C (period=7) runs every 7 ticks

---

## Part 9: Key Differences: RM vs EDF

### Rate Monotonic (RM)
```
✓ Static priorities (based on period)
✓ Period never changes
✓ Sort once when task added
✓ Predictable scheduling

Example:
Task A: Period=5 → Always priority 2
Task B: Period=3 → Always priority 1 (highest)
Task C: Period=7 → Always priority 3 (lowest)
```

### Earliest Deadline First (EDF)
```
✓ Dynamic priorities (based on deadline)
✓ Deadline decrements each tick
✓ Sort frequently (every tick)
✓ More complex but theoretically optimal

Example:
Tick 0: A(D=5), B(D=3), C(D=7) → B runs
Tick 1: A(D=4), B(D=2), C(D=6) → B runs
Tick 2: A(D=3), C(D=5)         → A runs
Tick 3: A(D=2), C(D=4), B(D=3) → A runs (newly respawned B)
```

---

## Part 10: Critical Implementation Details

### Interrupt Handling

**All scheduler operations MUST be atomic**:
```c
void scheduler(void) {
    DISABLE();                          // Critical section
    respawn_periodic_tasks();
    scheduler_RM();  // or scheduler_EDF()
    // ⚠️ Note: dispatch() longjmps away, so ENABLE() may not be called
}
```

### Context Switching

```c
static void dispatch(thread next) {
    if (current != NULL) {
        if (setjmp(current->context) == 0) {    // Save current
            current = next;
            longjmp(next->context, 1);          // Jump to next
        }
        // If we return here, we're resuming the original current
    }
}
```

### DoneQ Management

**When task completes**:
```c
// Inside spawn (when thread function returns)
enqueue(current, &doneQ);      // Move to doneQ (NOT freeQ for periodic tasks)
dispatch(dequeue(&readyQ));    // Switch to next ready task
```

---

## Summary

### Complete Flow in a4p2.c

1. **System Initialization**:
   - `main()` spawns three periodic tasks via `spawnWithDeadline()`
   - Tasks created with deadlines: Task0(5), Task1(3), Task2(7)
   - Timer interrupts configured via `initTimerInterrupts()`
   - Main thread spins in infinite loop

2. **spawnWithDeadline**: 
   - Creates periodic task with deadline and relative period
   - Sets up context with `setjmp/SETSTACK`
   - Enqueues to readyQ (unsorted initially)

3. **Timer Interrupt Trigger**:
   - Every tick (~1 second with Load=0xF3C)
   - `interrupt_vector()` calls `scheduler()`

4. **scheduler → scheduler_RM**:
   - First: `respawn_periodic_tasks()` - checks doneQ for tasks ready to respawn
   - Then: `sort(&readyQ)` - sorts by `Rel_Period_Deadline` (RM priority)
   - Finally: `yield()` - round-robin context switch

5. **respawn_periodic_tasks**:
   - Uses absolute deadline comparison (`ticks >= Period_Deadline`)
   - Increments deadline for next period
   - Re-establishes context with `setjmp/SETSTACK`
   - Moves ready tasks back to readyQ

6. **Task Execution**:
   - Task runs until function returns
   - In a4p2.c, `computeSomething` uses busy-wait:
     - Captures `ticks` at start
     - Computes and displays result
     - Waits until `ticks` advances (ensures at least one tick elapsed)
   - Return handler enqueues to doneQ (not freeQ)
   - Dispatches next task from readyQ

7. **Periodic Cycle**:
   - Tasks execute once per period
   - After completion, wait in doneQ
   - At period boundary, respawned to readyQ
   - Continuously cycles through execution and respawn

The system continuously cycles through these steps, ensuring periodic tasks are executed at their designated frequencies while respecting real-time constraints.

---

## Appendix: Step-by-Step Implementation Guide

### Example Implementation: respawn_periodic_tasks()

**Actual Implementation** (from a4p2):

```c
void respawn_periodic_tasks(void) {
    thread p = dequeue(&doneQ);
    thread tempDoneQ = NULL;
    volatile int t = ticks;

    while (p != NULL) {
        // If the thread's deadline hasn't passed, move to the next one
        if ((unsigned int)t < p->Period_Deadline) {
            enqueue(p, &tempDoneQ);
            p = dequeue(&doneQ);
            continue;
        }
        // Update the period deadline for the thread
        p->Period_Deadline += p->Rel_Period_Deadline;

        // Try to set the thread's context for the next run
        if (setjmp(p->context) != 0) {
            ENABLE();
            current->function(current->arg);
            DISABLE();
            enqueue(current, &doneQ);

            // Dispatch the next thread from readyQ
            thread next = dequeue(&readyQ);
            if (next != NULL) {
                dispatch(next);
            }
            return;
        }
        SETSTACK(&p->context, &p->stack);
        // Enqueue the thread back into readyQ
        enqueue(p, &readyQ);
        // Move to the next thread
        p = dequeue(&doneQ);
    }
    // Update doneQ with threads not ready to run yet
    doneQ = tempDoneQ;
}
```

**Key Implementation Details**:
- Uses absolute deadline comparison (`ticks >= Period_Deadline`)
- Increments deadline: `Period_Deadline += Rel_Period_Deadline`
- Uses tempDoneQ to efficiently filter ready tasks
- Re-establishes context with setjmp and SETSTACK before respawning
- Each respawned task maintains its own context and execution state

### Example Implementation: sort() - Insertion Sort

```c
static void sort(thread *queue) {
    if (*queue == NULL || (*queue)->next == NULL) {
        return;  // Empty or single element, already sorted
    }
    
    thread sorted = NULL;  // Start with empty sorted list
    thread current = *queue;  // Process unsorted list
    
    while (current != NULL) {
        thread next = current->next;  // Save next before modifying
        
        // Find correct position in sorted list
        thread *sorted_ptr = &sorted;
        while (*sorted_ptr != NULL && 
               (*sorted_ptr)->Period_Deadline <= current->Period_Deadline) {
            sorted_ptr = &((*sorted_ptr)->next);
        }
        
        // Insert current before sorted_ptr
        current->next = *sorted_ptr;
        *sorted_ptr = current;
        
        current = next;
    }
    
    *queue = sorted;  // Update queue head
}
```

### Example Implementation: scheduler_RM()

**Actual Implementation** (from a4p2):

```c
static void scheduler_RM(void){
    DISABLE();
    respawn_periodic_tasks();  // FIRST: respawn tasks
    sort(&readyQ);             // THEN: sort by priority
    yield();                   // Round-robin dispatch
    ENABLE();
}
```

**Note**: The a4p2 implementation uses a simplified approach:
- Always calls `respawn_periodic_tasks()` first
- Always sorts `readyQ` by `Rel_Period_Deadline` (RM priority)
- Uses standard `yield()` for dispatch (round-robin)
- No explicit priority comparison or preemption logic

**Alternative Implementation** (Priority-based preemption):

```c
static void scheduler_RM(void) {
    if (readyQ == NULL) {
        return;  // No ready tasks
    }
    
    // Head of readyQ has highest priority (shortest period)
    thread next = readyQ;
    
    if (current == &initp) {
        // Always switch from main thread
        dispatch(next);
        return;
    }
    
    // Check if preemption needed
    if (current->Rel_Period_Deadline <= next->Rel_Period_Deadline) {
        // Current task has higher or equal priority
        return;  // Continue executing current task
    }
    
    // Preempt: switch to higher priority task
    readyQ = readyQ->next;
    enqueue(current, &readyQ);
    sort(&readyQ);
    dispatch(next);
}
```

### Example Implementation: scheduler_EDF()

```c
static void scheduler_EDF(void) {
    if (readyQ == NULL) {
        return;
    }
    
    // Update all task deadlines
    thread t = readyQ;
    while (t != NULL) {
        if (t->Period_Deadline > 0) {
            t->Period_Deadline--;
        }
        t = t->next;
    }
    
    // Sort by deadline after update
    sort(&readyQ);
    
    // Select highest priority (earliest deadline)
    thread next = readyQ;
    
    if (current == &initp) {
        dispatch(next);
        return;
    }
    
    // Check if preemption needed
    if (current->Period_Deadline <= next->Period_Deadline) {
        return;  // Continue current
    }
    
    // Preempt
    readyQ = readyQ->next;
    enqueue(current, &readyQ);
    sort(&readyQ);
    dispatch(next);
}
```

