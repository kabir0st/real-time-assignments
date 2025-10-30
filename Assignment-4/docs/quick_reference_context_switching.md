# Quick Reference: Preemptive Context Switching Fix

## TL;DR - The Problem

**Original Issue**: Threads freeze when timer interrupts trigger context switches (without explicit `yield()` calls)

**Root Cause**: `setjmp/longjmp` don't save CPU mode. When switching from IRQ handler, threads incorrectly run in IRQ mode with IRQ stack → system freeze

**The Fix**: Explicitly exit IRQ mode and enter SVC mode before/after context switches

---

## What Changed

### New Files

1. **`a4p1/lib/context_switch_simple.s`** - Assembly functions for mode switching
2. **`a4p1/lib/context_switch.h`** - Header for full context implementation (optional)
3. **`a4p1/lib/context_switch.s`** - Full context switch implementation (optional)

### Modified Files

1. **`a4p1/lib/tinythreads.c`**
   - Added calls to `exit_irq_mode()` before `longjmp()`
   - Added calls to `enter_svc_mode_for_thread()` after context switches

2. **`a4p1/Makefile`**
   - Added `lib/context_switch_simple.o` to OBJS
   - Added rule to assemble `.s` files

---

## The Fix in Code

### Before (Broken)
```c
static void dispatch(thread next) {
    if (next != NULL) {
        if (setjmp(current->context) == 0) {
            current = next;
            longjmp(next->context, 1);  // ❌ Still in IRQ mode!
        }
    }
}
```

### After (Fixed)
```c
static void dispatch(thread next) {
    if (next != NULL) {
        if (setjmp(current->context) == 0) {
            current = next;
            exit_irq_mode();              // ✅ Exit IRQ, enter SVC mode
            longjmp(next->context, 1);    // ✅ Now safe!
        }
    }
    enter_svc_mode_for_thread();          // ✅ Ensure correct mode
}
```

---

## How It Works

### Normal Execution Flow (Without Fix)
```
Timer IRQ → IRQ mode → scheduler() → yield() → dispatch()
→ longjmp() [still in IRQ mode!] → Thread runs in IRQ mode ❌
→ FREEZE
```

### Fixed Execution Flow
```
Timer IRQ → IRQ mode → scheduler() → yield() → dispatch()
→ exit_irq_mode() → SVC mode ✅ → longjmp() → Thread runs in SVC mode ✅
→ enter_svc_mode_for_thread() → Interrupts re-enabled ✅
→ WORKS!
```

---

## Key Assembly Functions

### exit_irq_mode()
**Purpose**: Transition from IRQ mode to SVC mode  
**When**: Before longjmp in context switch  
**Effect**: Changes CPU mode bits in CPSR, switches to SVC stack

```assembly
mrs r0, cpsr                    # Read current mode
bic r0, r0, #0x1F              # Clear mode bits
orr r0, r0, #0x13              # Set SVC mode
msr cpsr_c, r0                 # Apply change
```

### enter_svc_mode_for_thread()
**Purpose**: Ensure thread runs in SVC mode with interrupts enabled  
**When**: After context switch completes  
**Effect**: Verifies SVC mode, re-enables interrupts

```assembly
mrs r0, cpsr
bic r0, r0, #0x1F              # Clear mode bits
orr r0, r0, #0x13              # Set SVC mode
bic r0, r0, #0x80              # Enable IRQs
msr cpsr_c, r0
```

---

## Testing

### Build
```bash
cd a4p1
make clean
make
```

### Expected Output (UART)
```
initTimerInterrupts
T0: 2
T1: 2
ticks: 1
T2: 2
ticks: 2
T3: 2
ticks: 3
T0: 7
ticks: 4
T1: 7
...
```

### What to Look For
✅ All four threads execute in rotation  
✅ ticks counter increments (timer interrupts firing)  
✅ No freezing or hanging  
✅ Threads switch automatically without calling yield()

---

## Troubleshooting

### Still Freezes?
1. Check `context_switch_simple.o` was compiled
2. Verify functions are linked (no linker errors)
3. Add debug output in `exit_irq_mode()` calls

### Only One Thread Runs?
1. Check timer interrupt is configured correctly
2. Verify interrupts are enabled (`ENABLE()` called)
3. Check readyQ has all threads

### Random Crashes?
1. Increase `STACKSIZE` if stack overflow
2. Check for shared state without synchronization
3. Verify stack pointers are set correctly in `spawn()`

---

## ARM Processor Modes (Quick Reference)

| Mode | Bits | Usage           | Our Stack | Status         |
|------|------|-----------------|-----------|----------------|
| IRQ  | 0x12 | Interrupts      | 0x7000    | Temporary only |
| SVC  | 0x13 | OS/Threads      | 0x8000    | Normal threads |

**Rule**: Threads must run in SVC mode, NOT IRQ mode!

---

## Why This Matters

### Cooperative Multitasking (Old, Working)
- Threads call `yield()` voluntarily
- Always in SVC mode when calling `yield()`
- setjmp/longjmp works fine

### Preemptive Multitasking (New, Was Broken)
- Timer interrupts force thread switches
- Interrupt handler is in IRQ mode
- setjmp/longjmp doesn't handle mode switch
- **Fix required**: Explicit mode management

---

## Related Documentation

- Full details: `docs/preemptive_context_switching.md`
- Context switch visualization: See "Visual Representation" section
- ARM modes explained: See "Understanding ARM Processor Modes" section

---

## Summary

**Problem**: CPU mode not preserved during interrupt-triggered context switch  
**Solution**: Manually manage mode transitions with assembly helpers  
**Result**: Preemptive multitasking now works correctly

**Key Insight**: User-space primitives (setjmp/longjmp) need kernel-level support (mode management) for preemptive scheduling.

