# Where does IRQ returns after context switching ? 

This is a **critical concept** in understanding preemptive multitasking with context switching. Let me trace through exactly what happens:


## The Magic of `longjmp()` - It NEVER Returns!

Here's the critical insight: **When a context switch happens, the ISR (interrupt_vector) NEVER returns normally!** 

Let me trace through what happens:

### Scenario: Thread A Running → Context Switch to Thread B

```
1. Thread A executing (normal code)
   ├─ Stack: Thread A's stack
   ├─ PC: somewhere in Thread A's code
   └─ Interrupts: ENABLED

2. Timer Interrupt Fires ⚡
   ├─ CPU hardware automatically:
   │  ├─ Disables interrupts
   │  ├─ Saves PC, CPSR to IRQ mode registers
   │  ├─ Switches to IRQ mode & IRQ stack (0x7000)
   │  └─ Jumps to interrupt_vector()
   │
   └─ Call Stack (on IRQ stack):
      interrupt_vector() ← WE ARE HERE
```

```c
void __attribute__((interrupt("IRQ"))) interrupt_vector(void)
{
    RPI_GetArmTimer()->IRQClear = 1;
    ticks++;
    scheduler();  // ← Call scheduler
    // ← COMMENT: "we may not return here"
}
```

```
3. Inside interrupt_vector() → scheduler() → scheduler_RR() → dispatch()
   
   Call Stack (still on IRQ stack 0x7000):
   ┌─────────────────────────┐
   │ dispatch()              │ ← TOP
   ├─────────────────────────┤
   │ scheduler_RR()          │
   ├─────────────────────────┤
   │ scheduler()             │
   ├─────────────────────────┤
   │ interrupt_vector()      │
   └─────────────────────────┘
```

```c
static void dispatch(thread next) {  // next = Thread B
    if (setjmp(current->context) == 0) {  
        // Saves Thread A's registers (r4-r11, sp, lr)
        // Returns 0 on first call
        
        current = next;  // current = Thread B
        longjmp(next->context, 1);  // ← JUMP to Thread B!
        // ⚠️ THIS LINE NEVER EXECUTES! ⚠️
    }
}
```

### What `longjmp()` Does:

```
4. longjmp(Thread B's context, 1) executes:
   ├─ Restores Thread B's registers from context:
   │  ├─ r4-r11 (saved registers)
   │  ├─ sp  ← Thread B's stack pointer!
   │  └─ lr  ← Thread B's return address
   │
   ├─ Performs: mov pc, lr  (jump to saved address)
   │
   └─ Returns value 1 to the setjmp that saved Thread B's context
```

**KEY POINT:** After `longjmp()`, we're **no longer on the IRQ stack**! We're now on **Thread B's stack**, and execution continues from wherever Thread B was previously interrupted.

### Where Does Execution Go?

Thread B could have been saved in two places:

#### Case 1: Thread B was previously interrupted by scheduler
If Thread B was running and got interrupted:
```c
// Thread B was here when timer fired last time
computeSomethingForever(seg) {
    for(...) {
        value = iexp(...);  // ← Thread B was executing here
        printf_at_seg(...);
        // Timer interrupt fired here!
    }
}

// After context switch in dispatch():
// dispatch() calls longjmp(Thread B's context)
// → Execution JUMPS back to Thread B
// → setjmp in dispatch() returns 1 (from the PREVIOUS interrupt)
// → dispatch() exits normally (this time!)
// → Returns to scheduler_RR()
// → Returns to scheduler()
// → Returns to interrupt_vector()
// → interrupt_vector() executes IRQ return
// → CPU restores Thread B's PC, re-enables interrupts
// → Thread B continues: printf_at_seg(...);
```

Wait, I need to reconsider this. Let me trace more carefully...

Actually, the key is understanding where Thread B's context was saved.

When Thread B was PREVIOUSLY interrupted:
1. Thread B running
2. Timer interrupt
3. Enter interrupt_vector → scheduler → scheduler_RR → dispatch
4. In dispatch: `setjmp(current->context) == 0` saves Thread B's context
5. Then longjmp to Thread C

So Thread B's saved context has:
- Stack pointer pointing to Thread B's stack
- PC pointing to the instruction AFTER the setjmp call in dispatch()
- All the registers

When we later do `longjmp(Thread B's context, 1)`:
- It restores all those registers
- Makes setjmp return 1 (instead of 0)
- Execution continues in dispatch() but with Thread B's stack
</thinking>

```
5. After longjmp(), execution is now in Thread B's context:
   
   ├─ Stack: Thread B's stack (NOT IRQ stack anymore!)
   ├─ setjmp() in dispatch() returns 1 (not 0)
   │  └─ Because longjmp returns to where setjmp was called
   │
   └─ Call Stack (now on Thread B's stack):
      ┌─────────────────────────┐
      │ dispatch()              │ ← setjmp returns 1
      ├─────────────────────────┤
      │ scheduler_RR()          │ ← Will return normally
      ├─────────────────────────┤
      │ scheduler()             │ ← Will return normally  
      ├─────────────────────────┤
      │ interrupt_vector()      │ ← Will return normally
      ├─────────────────────────┤
      │ computeSomethingForever()│ ← Thread B resumes here!
      └─────────────────────────┘
```

### Let me show this more clearly:

```c
// dispatch() when switching FROM Thread A TO Thread B:
static void dispatch(thread next) {  // next = Thread B
    if (setjmp(current->context) == 0) {  // current = Thread A
        // ✓ Saves Thread A's state
        // ✓ Returns 0 (first call)
        
        current = next;  // current = Thread B
        longjmp(next->context, 1);  
        // ☠️ NEVER RETURNS! Execution jumps away!
    }
    // ☠️ Never reaches here (when context switching)
}

// BUT Thread B's context was saved PREVIOUSLY when Thread B was running:
// From when Thread B was interrupted before:
static void dispatch(thread next) {  // next was Thread C
    if (setjmp(current->context) == 0) {  // current was Thread B
        // This saved Thread B's context including:
        // - Stack pointer → Thread B's stack
        // - PC → right after this setjmp call
        // - All registers
        
        current = next;
        longjmp(next->context, 1);  // Switched to Thread C
    }
}

// NOW when we do longjmp(Thread B's context, 1):
static void dispatch(thread next) {
    if (setjmp(current->context) == 0) {  // Returns 1 (from longjmp!)
        // Skipped!
    }
    // ✓ Continues here! On Thread B's stack!
}  // Returns normally

// Then the call stack unwinds normally:
→ scheduler_RR() returns normally
→ scheduler() returns normally  
→ interrupt_vector() returns normally (with IRQ return instruction)
→ CPU restores Thread B's PC and CPSR
→ Thread B continues executing from where it was interrupted!
```

## Visual Timeline:

```
Thread A Running
    ↓
Timer IRQ → interrupt_vector() [IRQ stack]
    ↓
scheduler() [IRQ stack]
    ↓
scheduler_RR() [IRQ stack]
    ↓
dispatch() [IRQ stack]
    ├─ setjmp(Thread A) → saves A's context
    └─ longjmp(Thread B) → TELEPORT! ⚡
              ↓
              ↓ [Now on Thread B's stack!]
              ↓
dispatch() returns [Thread B's stack]
    ↓
scheduler_RR() returns [Thread B's stack]
    ↓
scheduler() returns [Thread B's stack]
    ↓
interrupt_vector() returns [Thread B's stack]
    ↓ [IRQ return instruction]
    ↓
Thread B continues from where it was interrupted
```

## The Answer to Your Question:

**When context switch happens:**
- The ISR code (interrupt_vector) **DOES eventually return**
- But it returns **on a different stack** (Thread B's stack, not the IRQ stack)
- It returns **to Thread B's code**, not to Thread A
- The `longjmp()` effectively "transplants" the entire call stack from the IRQ stack to Thread B's stack

**That's why the comment says "may not return here"** — because after `scheduler()` is called, we might `longjmp()` away to another thread, and the return path continues on that thread's stack, eventually returning to that thread's interrupted code, not back to where Thread A was running!

This is the magic of cooperative multitasking using `setjmp/longjmp` in an interrupt-driven scheduler! 🎩✨