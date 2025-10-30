# Visual Guide: Context Switching Fix

This document provides visual diagrams to understand how the context switching fix works.

---

## Overview: The Problem vs The Solution

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         WITHOUT FIX (BROKEN)                            │
└─────────────────────────────────────────────────────────────────────────┘

Thread A Running                Timer IRQ                    FREEZE!
┌──────────────┐             ┌──────────────┐           ┌──────────────┐
│              │             │              │           │              │
│  SVC Mode    │   IRQ! →   │  IRQ Mode    │  longjmp  │  IRQ Mode    │
│  Stack:0x8000│             │  Stack:0x7000│    →      │  Stack:0x7000│
│              │             │              │           │              │
│  Thread A    │             │ scheduler()  │           │  Thread B    │
│  running ✓   │             │   yield()    │           │  WRONG! ✗    │
│              │             │   dispatch() │           │              │
└──────────────┘             └──────────────┘           └──────────────┘
                                     │                         │
                                     └─────────────────────────┘
                                     Thread runs in IRQ mode!
                                     Uses wrong stack! → FREEZE


┌─────────────────────────────────────────────────────────────────────────┐
│                         WITH FIX (WORKING)                              │
└─────────────────────────────────────────────────────────────────────────┘

Thread A Running          Timer IRQ              Mode Switch         Success!
┌──────────────┐       ┌──────────────┐       ┌──────────────┐  ┌──────────────┐
│              │       │              │       │              │  │              │
│  SVC Mode    │ IRQ!  │  IRQ Mode    │ exit_ │  SVC Mode    │  │  SVC Mode    │
│  Stack:0x8000│  →    │  Stack:0x7000│  irq_ │  Stack:0x8000│→ │  Stack:0x8000│
│              │       │              │ mode()│              │  │              │
│  Thread A    │       │ scheduler()  │   →   │  dispatch()  │  │  Thread B    │
│  running ✓   │       │   yield()    │       │   longjmp()  │  │  running ✓   │
│              │       │              │       │              │  │              │
└──────────────┘       └──────────────┘       └──────────────┘  └──────────────┘
```

---

## CPU State Transitions

### Without Fix (Broken)

```
State 1: Normal Execution
┌─────────────────────────────────────┐
│ CPSR:  0x00000013  (SVC mode)       │
│ SP:    0x00007FF0  (SVC stack)      │
│ PC:    Thread A code                │
│ Mode:  Supervisor (0x13)            │
└─────────────────────────────────────┘
               │
               │ Timer IRQ fires
               ↓
State 2: In IRQ Handler
┌─────────────────────────────────────┐
│ CPSR:  0x00000092  (IRQ mode)       │
│ SP:    0x00006FF0  (IRQ stack)      │
│ PC:    interrupt_vector()           │
│ Mode:  IRQ (0x12)                   │
│ SPSR:  0x00000013  (saved SVC)      │
└─────────────────────────────────────┘
               │
               │ scheduler() → yield() → dispatch()
               ↓
State 3: After longjmp (WRONG!)
┌─────────────────────────────────────┐
│ CPSR:  0x00000092  (STILL IRQ!)  ✗  │
│ SP:    0x00006FF0  (IRQ stack)   ✗  │
│ PC:    Thread B code                │
│ Mode:  IRQ (0x12)  ← WRONG!         │
└─────────────────────────────────────┘
               │
               │ Thread tries to run
               ↓
           FREEZE / CRASH
```

### With Fix (Working)

```
State 1: Normal Execution
┌─────────────────────────────────────┐
│ CPSR:  0x00000013  (SVC mode)       │
│ SP:    0x00007FF0  (SVC stack)      │
│ PC:    Thread A code                │
│ Mode:  Supervisor (0x13)            │
└─────────────────────────────────────┘
               │
               │ Timer IRQ fires
               ↓
State 2: In IRQ Handler
┌─────────────────────────────────────┐
│ CPSR:  0x00000092  (IRQ mode)       │
│ SP:    0x00006FF0  (IRQ stack)      │
│ PC:    interrupt_vector()           │
│ Mode:  IRQ (0x12)                   │
└─────────────────────────────────────┘
               │
               │ scheduler() → yield() → dispatch()
               ↓
State 3: After exit_irq_mode() (FIXED!)
┌─────────────────────────────────────┐
│ CPSR:  0x000000D3  (SVC mode!)   ✓  │
│ SP:    0x00007FF0  (SVC stack!)  ✓  │
│ PC:    Still in dispatch()          │
│ Mode:  Supervisor (0x13)  ← FIXED!  │
└─────────────────────────────────────┘
               │
               │ longjmp(Thread B)
               ↓
State 4: Thread B Running
┌─────────────────────────────────────┐
│ CPSR:  0x00000053  (SVC, IRQs on) ✓ │
│ SP:    0x00007FD0  (Thread B stack) │
│ PC:    Thread B code                │
│ Mode:  Supervisor (0x13)  ← CORRECT!│
└─────────────────────────────────────┘
               │
               ↓
        Execution continues normally!
```

---

## Stack Layout

### Memory Map

```
High Memory
    ↑
    │
0x8000 ┌─────────────────────┐ ← SVC Stack Top
       │                     │
       │   SVC Mode Stack    │   ← Threads run here
       │   (Application)     │
       │                     │
0x7FF0 │ Thread A stack      │ ← Thread A SP
       │ ...                 │
0x7FD0 │ Thread B stack      │ ← Thread B SP
       │ ...                 │
0x7000 └─────────────────────┘ ← SVC Stack Bottom / IRQ Stack Top
       ┌─────────────────────┐
       │                     │
       │   IRQ Mode Stack    │   ← Interrupt handler uses this
       │                     │
0x6FF0 │ IRQ handler frames  │ ← IRQ SP during interrupt
       │ ...                 │
0x6000 └─────────────────────┘ ← IRQ Stack Bottom
    │
    ↓
Low Memory
```

### Without Fix: Stack Confusion

```
During IRQ (WRONG):
┌────────────────────────────────────────────────────────────┐
│  Thread expects to use SVC stack (0x8000 region)           │
│  BUT CPU is in IRQ mode                                    │
│  SO it uses IRQ stack (0x7000 region)                      │
│  Thread's local variables → written to IRQ stack           │
│  Next IRQ → overwrites thread data                         │
│  CORRUPTION!                                               │
└────────────────────────────────────────────────────────────┘

Visual:
SVC Stack (Thread expects)     IRQ Stack (What it actually uses)
0x7FF0: [Thread B data]        0x7000: [IRQ frame]
0x7FE0: [unused]               0x6FF0: [Thread B writes here!] ✗
0x7FD0: [unused]               0x6FE0: [Wrong location!] ✗
```

### With Fix: Correct Stack Usage

```
After exit_irq_mode() (CORRECT):
┌────────────────────────────────────────────────────────────┐
│  exit_irq_mode() changes CPSR mode bits to SVC             │
│  ARM CPU automatically switches SP to SVC stack register   │
│  Thread now uses correct stack (0x8000 region)             │
│  Thread's local variables → written to SVC stack           │
│  IRQ stack abandoned (will reset on next IRQ)              │
│  NO CORRUPTION!                                            │
└────────────────────────────────────────────────────────────┘

Visual:
SVC Stack (Correct!)           IRQ Stack (Abandoned, OK)
0x7FF0: [Thread A data]        0x7000: [Old IRQ frame]
0x7FE0: [Thread A frames]      0x6FF0: [Abandoned]
0x7FD0: [Thread B data] ✓      0x6FE0: [Not used]
0x7FC0: [Thread B writes] ✓    0x6FD0: [Not used]
```

---

## Execution Flow Diagram

### Complete Preemptive Context Switch

```
┌─────────────────────────────────────────────────────────────────────────┐
│ Thread A: computeSomethingForever(0)                                    │
├─────────────────────────────────────────────────────────────────────────┤
│ for(volatile uint32_t i=0; ; i++) {                                     │
│     value = iexp((i % 9) + 1);          ← Executing here                │
│     printf_at_seg(0, "T0: %d", value);    │                             │
│     ...                                   │                             │
│ }                                         │                             │
└───────────────────────────────────────────┼─────────────────────────────┘
                                            │
                    ┌───────────────────────┘
                    │ Timer expires! Hardware IRQ!
                    ↓
┌─────────────────────────────────────────────────────────────────────────┐
│ HARDWARE ACTIONS (Automatic)                                            │
├─────────────────────────────────────────────────────────────────────────┤
│ 1. LR_irq ← PC + 4          (Save return address)                       │
│ 2. SPSR_irq ← CPSR          (Save current CPSR)                         │
│ 3. CPSR mode ← IRQ (0x12)   (Enter IRQ mode)                            │
│ 4. SP ← IRQ stack           (Switch to IRQ stack at 0x7000)             │
│ 5. CPSR I-bit ← 1           (Disable interrupts)                        │
│ 6. PC ← IRQ vector          (Jump to interrupt handler)                 │
└─────────────────────────────────────────────────────────────────────────┘
                    │
                    ↓
┌─────────────────────────────────────────────────────────────────────────┐
│ interrupt_vector()                          [IRQ mode, 0x7000 stack]    │
├─────────────────────────────────────────────────────────────────────────┤
│ if (RPI_GetArmTimer()->MaskedIRQ) {                                     │
│     RPI_GetArmTimer()->IRQClear = 1;  // Clear interrupt                │
│     ticks++;                          // Increment counter              │
│     scheduler(); ──────────────────────┐                                │
│ }                                      │                                │
└────────────────────────────────────────┼─────────────────────────────────┘
                                         │
                    ┌────────────────────┘
                    ↓
┌─────────────────────────────────────────────────────────────────────────┐
│ scheduler()                                 [IRQ mode, 0x7000 stack]    │
├─────────────────────────────────────────────────────────────────────────┤
│ scheduler_RR(); ──────────────────────┐                                 │
└───────────────────────────────────────┼─────────────────────────────────┘
                                        │
                    ┌───────────────────┘
                    ↓
┌─────────────────────────────────────────────────────────────────────────┐
│ scheduler_RR()                              [IRQ mode, 0x7000 stack]    │
├─────────────────────────────────────────────────────────────────────────┤
│ yield(); ─────────────────────────────┐                                 │
└───────────────────────────────────────┼─────────────────────────────────┘
                                        │
                    ┌───────────────────┘
                    ↓
┌─────────────────────────────────────────────────────────────────────────┐
│ yield()                                     [IRQ mode, 0x7000 stack]    │
├─────────────────────────────────────────────────────────────────────────┤
│ if (readyQ != NULL) {                                                   │
│     thread p = dequeue(&readyQ);      // Get Thread B                   │
│     enqueue(current, &readyQ);        // Enqueue Thread A               │
│     dispatch(p); ─────────────────────┐                                 │
│ }                                     │                                 │
└───────────────────────────────────────┼─────────────────────────────────┘
                                        │
                    ┌───────────────────┘
                    ↓
┌─────────────────────────────────────────────────────────────────────────┐
│ dispatch(Thread B)                          [IRQ mode, 0x7000 stack]    │
├─────────────────────────────────────────────────────────────────────────┤
│ if (next != NULL) {                                                     │
│     if (setjmp(current->context) == 0) {    // Save Thread A context    │
│         current = next;                     // current → Thread B        │
│         exit_irq_mode(); ──────────────────┐  ★ THE FIX!               │
│     }                                      │                            │
│ }                                          │                            │
└────────────────────────────────────────────┼────────────────────────────┘
                                             │
                    ┌────────────────────────┘
                    ↓
┌─────────────────────────────────────────────────────────────────────────┐
│ exit_irq_mode()                             [IRQ mode → SVC mode]       │
├─────────────────────────────────────────────────────────────────────────┤
│ push {lr}                                                               │
│ mrs r0, cpsr              // r0 = 0x00000092 (IRQ mode)                 │
│ bic r0, r0, #0x1F        // r0 = 0x00000080 (clear mode bits)          │
│ orr r0, r0, #0x13        // r0 = 0x00000093 (set SVC mode)             │
│ orr r0, r0, #0xC0        // r0 = 0x000000D3 (keep IRQs disabled)       │
│ msr cpsr_c, r0           // ★ MODE CHANGED TO SVC! ★                   │
│ pop {pc}                 // Return                                      │
└─────────────────────────────────────────────────────────────────────────┘
    ★ CPU NOW IN SVC MODE ★
    ★ STACK AUTOMATICALLY SWITCHED TO 0x8000 ★
                    │
                    ↓
┌─────────────────────────────────────────────────────────────────────────┐
│ dispatch(Thread B) [continued]              [SVC mode, 0x8000 stack] ✓ │
├─────────────────────────────────────────────────────────────────────────┤
│         longjmp(next->context, 1); ────────┐  // Jump to Thread B       │
│     }                                      │                            │
│ }                                          │                            │
└────────────────────────────────────────────┼────────────────────────────┘
                                             │
                    ┌────────────────────────┘
                    ↓
┌─────────────────────────────────────────────────────────────────────────┐
│ Thread B: computeSomethingForever(1)        [SVC mode, 0x8000 stack] ✓ │
├─────────────────────────────────────────────────────────────────────────┤
│ for(volatile uint32_t i=0; ; i++) {                                     │
│     value = iexp((i % 9) + 1);                                          │
│     printf_at_seg(1, "T1: %d", value);  ← Resume here!                  │
│     ...                                                                 │
│ }                                                                       │
└─────────────────────────────────────────────────────────────────────────┘
                    │
                    ↓
              Thread B continues
              execution normally!
                  SUCCESS! ✓
```

---

## CPSR Register Breakdown

### CPSR (Current Program Status Register)

```
 31 30 29 28 27          8  7  6  5  4  3  2  1  0
┌──┬──┬──┬──┬────────────┬──┬──┬──┬──┬──┬──┬──┬──┐
│N │Z │C │V │ Reserved   │I │F │T │  Mode         │
└──┴──┴──┴──┴────────────┴──┴──┴──┴──┴──┴──┴──┴──┘
  │  │  │  │              │  │  │  └─────┬─────┘
  │  │  │  │              │  │  │        └──────── Mode bits [4:0]
  │  │  │  │              │  │  └────────────────── Thumb (T) bit
  │  │  │  │              │  └───────────────────── FIQ disable (F)
  │  │  │  │              └──────────────────────── IRQ disable (I)
  │  │  │  └─────────────────────────────────────── Overflow (V)
  │  │  └────────────────────────────────────────── Carry (C)
  │  └───────────────────────────────────────────── Zero (Z)
  └──────────────────────────────────────────────── Negative (N)
```

### Mode Bits Values

```
Mode       Binary      Hex    Description
────────   ─────────   ────   ──────────────────
User       0b10000     0x10   Normal user programs
FIQ        0b10001     0x11   Fast interrupt
IRQ        0b10010     0x12   Normal interrupt ← Problem mode!
SVC        0b10011     0x13   Supervisor       ← Target mode!
Abort      0b10111     0x17   Memory abort
Undefined  0b11011     0x1B   Undefined instruction
System     0b11111     0x1F   Privileged user
```

### Example CPSR Values

```
IRQ Mode, Interrupts Disabled:
┌─────────────────────────────────────┐
│ CPSR = 0x00000092                   │
│                                     │
│ Bits 7-0: 0b10010010 = 0x92        │
│   Bit 7 (I): 1 ← IRQs disabled     │
│   Bit 6 (F): 0                      │
│   Bit 5 (T): 0 ← ARM mode           │
│   Bits 4-0: 0b10010 = 0x12 ← IRQ   │
└─────────────────────────────────────┘

SVC Mode, Interrupts Disabled:
┌─────────────────────────────────────┐
│ CPSR = 0x000000D3                   │
│                                     │
│ Bits 7-0: 0b11010011 = 0xD3        │
│   Bit 7 (I): 1 ← IRQs disabled     │
│   Bit 6 (F): 1 ← FIQs disabled     │
│   Bit 5 (T): 0 ← ARM mode           │
│   Bits 4-0: 0b10011 = 0x13 ← SVC   │
└─────────────────────────────────────┘

SVC Mode, Interrupts Enabled:
┌─────────────────────────────────────┐
│ CPSR = 0x00000053                   │
│                                     │
│ Bits 7-0: 0b01010011 = 0x53        │
│   Bit 7 (I): 0 ← IRQs enabled      │
│   Bit 6 (F): 1 ← FIQs disabled     │
│   Bit 5 (T): 0 ← ARM mode           │
│   Bits 4-0: 0b10011 = 0x13 ← SVC   │
└─────────────────────────────────────┘
```

---

## Mode Transition Assembly

### exit_irq_mode() Step-by-Step

```assembly
exit_irq_mode:
    ; Entry state: IRQ mode, CPSR = 0x00000092
    
    push {lr}
    ; Stack (IRQ): [return address]
    
    mrs r0, cpsr
    ; r0 = 0x00000092
    ;      0b00000000000000000000000010010010
    ;        ││││││││││││││││││││││││└┴┴┴┴┴── Mode: 0b10010 (IRQ)
    ;        │││││││││││││││││││││││└──────── Thumb: 0
    ;        ││││││││││││││││││││││└───────── FIQ disable: 0
    ;        │││││││││││││││││││││└────────── IRQ disable: 1
    
    bic r0, r0, #0x1F
    ; r0 = 0x00000080
    ;      0b00000000000000000000000010000000
    ;      Cleared bits 4-0 (mode bits)
    
    orr r0, r0, #0x13
    ; r0 = 0x00000093
    ;      0b00000000000000000000000010010011
    ;                                   └┴┴┴┴┴── Mode: 0b10011 (SVC)
    
    orr r0, r0, #0xC0
    ; r0 = 0x000000D3
    ;      0b00000000000000000000000011010011
    ;                               ││└┴┴┴┴┴── Mode: SVC
    ;                               │└──────── FIQ disable: 1
    ;                               └───────── IRQ disable: 1
    
    msr cpsr_c, r0
    ; ★★★ MODE CHANGED! ★★★
    ; CPSR = 0x000000D3
    ; CPU now in SVC mode
    ; SP automatically switches to SVC stack pointer
    ; LR automatically switches to SVC link register
    
    pop {pc}
    ; Return (now in SVC mode!)
```

---

## Summary Diagram

```
╔═════════════════════════════════════════════════════════════════════╗
║                      THE COMPLETE FIX                               ║
╠═════════════════════════════════════════════════════════════════════╣
║                                                                     ║
║  Problem:  IRQ mode → longjmp → Thread in IRQ mode → FREEZE        ║
║                                                                     ║
║  Solution: IRQ mode → exit_irq_mode() → SVC mode →                 ║
║            → longjmp → Thread in SVC mode → SUCCESS                ║
║                                                                     ║
║  Key:      Explicitly manage CPU mode during context switch        ║
║                                                                     ║
╚═════════════════════════════════════════════════════════════════════╝
```

---

**Remember**: setjmp/longjmp save registers, but NOT CPU mode. For kernel-level preemptive multitasking, you must manage modes explicitly!

