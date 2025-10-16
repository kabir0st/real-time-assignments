
## Timer Interrupt Flow

### High-Level Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                    SYSTEM STARTUP & EXECUTION                     │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│  1. BOOT SEQUENCE (startup.c)                                    │
│     ├─ Entry point at 0x8000                                     │
│     ├─ Setup interrupt vector table at 0x0000                    │
│     ├─ Configure IRQ mode stack (0x7000)                         │
│     ├─ Configure Supervisor mode stack (0x8000)                  │
│     └─ Call SystemInit() → main()                                │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│  2. MAIN PROGRAM (a4p1.c)                                        │
│     ├─ Initialize PiFace display                                 │
│     ├─ Call initTimerInterrupts()                                │
│     ├─ Spawn 3 threads (computeSomethingForever)                 │
│     └─ Run main thread (computeSomethingForever)                 │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│  3. TIMER INTERRUPT SETUP (a4p1.c: initTimerInterrupts())       │
│     ├─ Enable ARM Timer IRQ in interrupt controller             │
│     ├─ Set Timer Load value (0xF3C ≈ 1 second)                  │
│     ├─ Configure Timer Control Register:                         │
│     │   • 23-bit counter mode                                    │
│     │   • Enable timer                                           │
│     │   • Enable interrupts                                      │
│     │   • Prescale by 256                                        │
│     └─ Enable global interrupts (CPSIE i)                        │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
        ┌─────────────────────────────────────────┐
        │   APPLICATION RUNNING (main threads)     │
        │   computeSomethingForever() executing    │
        └─────────────────────────────────────────┘
                              │
                              │ Timer counts down...
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│  4. HARDWARE TIMER INTERRUPT                                     │
│     ARM Timer peripheral counts down to 0                        │
│     └─ Generates IRQ signal to ARM CPU                           │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│  5. CPU INTERRUPT HANDLING                                       │
│     ├─ CPU receives IRQ signal                                   │
│     ├─ Save current CPU state (CPSR, PC, registers)             │
│     ├─ Switch to IRQ mode                                        │
│     ├─ Look up interrupt vector at 0x0018                        │
│     └─ Jump to interrupt_vector()                                │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│  6. INTERRUPT SERVICE ROUTINE (rpi-interrupts.c)                 │
│     interrupt_vector():                                          │
│     ├─ Check if ARM Timer caused interrupt (MaskedIRQ)          │
│     ├─ Clear timer interrupt flag (IRQClear = 1)                │
│     ├─ Increment global 'ticks' counter                          │
│     └─ Return from interrupt (restore CPU state)                 │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
        ┌─────────────────────────────────────────┐
        │   RETURN TO APPLICATION                  │
        │   Resume computeSomethingForever()       │
        └─────────────────────────────────────────┘
                              │
                              │ Cycle repeats...
                              │
                              ▼
```

## Detailed Timer Interrupt Call Chain

```
HARDWARE LAYER:
┌──────────────────────────────────────────────────────────────┐
│  ARM Timer Peripheral (RPI_ARMTIMER_BASE: PERIPHERAL_BASE    │
│                        + 0xB400)                              │
│                                                               │
│  Timer Registers:                                            │
│  ├─ Load: 0xF3C (initial count value)                       │
│  ├─ Value: Current counter (counts down)                     │
│  ├─ Control: Configuration flags                             │
│  ├─ IRQClear: Write to clear interrupt                       │
│  ├─ RAWIRQ: Raw interrupt status                             │
│  └─ MaskedIRQ: Masked interrupt status                       │
│                                                               │
│  Operation:                                                   │
│  Value counts down: 0xF3C → 0xF3B → ... → 1 → 0             │
│  When Value == 0:                                            │
│  ├─ Reload Value from Load register                          │
│  ├─ Set RAWIRQ flag                                          │
│  └─ If interrupts enabled: Assert IRQ line to CPU            │
└──────────────────────────────────────────────────────────────┘
                              │ IRQ Signal
                              ▼
┌──────────────────────────────────────────────────────────────┐
│  BCM2835 Interrupt Controller (RPI_INTERRUPT_CONTROLLER_BASE)│
│                                                               │
│  ├─ Receives IRQ from ARM Timer                              │
│  ├─ Checks if ARM Timer IRQ is enabled                       │
│  │   (Enable_Basic_IRQs & RPI_BASIC_ARM_TIMER_IRQ)          │
│  └─ Routes interrupt to ARM CPU IRQ line                     │
└──────────────────────────────────────────────────────────────┘
                              │ CPU IRQ
                              ▼
SOFTWARE LAYER:
┌──────────────────────────────────────────────────────────────┐
│  ARM CPU Exception/Interrupt Vector Table (@ 0x0000)         │
│                                                               │
│  Offset  │ Exception Type        │ Handler Address           │
│  ────────┼───────────────────────┼─────────────────────────  │
│  0x0000  │ Reset                 │ → _reset_                 │
│  0x0004  │ Undefined Instruction │ → undefined_instruction_* │
│  0x0008  │ Software Interrupt    │ → software_interrupt_*    │
│  0x000C  │ Prefetch Abort        │ → prefetch_abort_vector   │
│  0x0010  │ Data Abort            │ → data_abort_vector       │
│  0x0014  │ (Unused)              │ → _reset_                 │
│  0x0018  │ IRQ Interrupt      ◄──┼─→ interrupt_vector()      │
│  0x001C  │ FIQ Interrupt         │ → fast_interrupt_vector   │
│                                                               │
│  Each entry: "ldr pc, [address]"                             │
│  Loads handler address and jumps to it                       │
└──────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌──────────────────────────────────────────────────────────────┐
│  interrupt_vector() - ISR (lib/rpi-interrupts.c:93)         │
│                                                               │
│  void __attribute__((interrupt("IRQ"))) interrupt_vector()   │
│  {                                                            │
│      // Check if ARM Timer caused this interrupt             │
│      if (RPI_GetArmTimer()->MaskedIRQ) {                     │
│          // Clear the interrupt flag                         │
│          RPI_GetArmTimer()->IRQClear = 1;                    │
│                                                               │
│          // Increment global tick counter                    │
│          ticks++;  // Used by application for timing         │
│      }                                                        │
│  }                                                            │
│                                                               │
│  __attribute__((interrupt("IRQ"))):                          │
│  ├─ Compiler generates special prologue/epilogue            │
│  ├─ Saves/restores registers automatically                   │
│  └─ Returns using special instruction (movs pc, lr)          │
└──────────────────────────────────────────────────────────────┘
                              │
                              ▼
        Return to interrupted application code
```

## Key Function Call Flow

### Initialization Sequence

```
1. Power On / Reset
   │
   ├─► Entry (startup.c:67)
   │   └─ Jump table setup at 0x8000
   │
   ├─► _reset_ (startup.c:86)
   │   ├─ Determine CPU mode (CPSR)
   │   ├─ Park non-primary cores (multicore)
   │   ├─ Setup interrupt vector table (0x0000)
   │   │   Copy from 0x8000 → 0x0000
   │   ├─ Setup IRQ stack: sp = 0x7000
   │   ├─ Setup SVC stack: sp = 0x8000
   │   ├─ Enable L1 cache & branch prediction
   │   └─ Call SystemInit()
   │
   ├─► SystemInit() (startup.c:219)
   │   ├─ Clear BSS section
   │   ├─ Call static constructors
   │   └─ Call main()
   │
   ├─► main() (a4p1.c:114)
   │   ├─ piface_init()
   │   ├─ piface_puts("DT8025 - A4P1")
   │   ├─ RPI_WaitMicroSeconds(2000000)
   │   ├─ piface_clear()
   │   │
   │   ├─► initTimerInterrupts() (a4p1.c:71)
   │   │   ├─ RPI_EnableARMTimerInterrupt() (a4p1.c:66)
   │   │   │   └─ Enable in interrupt controller
   │   │   │
   │   │   ├─ RPI_GetArmTimer()->Load = 0xF3C
   │   │   │   Timer frequency = Clk/256 * 0x400
   │   │   │   0xF3C ≈ 1 second interval
   │   │   │
   │   │   ├─ RPI_GetArmTimer()->Control = 
   │   │   │   RPI_ARMTIMER_CTRL_23BIT |      // 23-bit counter
   │   │   │   RPI_ARMTIMER_CTRL_ENABLE |     // Enable timer
   │   │   │   RPI_ARMTIMER_CTRL_INT_ENABLE | // Enable interrupts
   │   │   │   RPI_ARMTIMER_CTRL_PRESCALE_256 // Prescaler = 256
   │   │   │
   │   │   └─ ENABLE()  // cpsie i (enable IRQ interrupts)
   │   │
   │   ├─► spawn(computeSomethingForever, 0) (tinythreads.c:148)
   │   ├─► spawn(computeSomethingForever, 1)
   │   ├─► spawn(computeSomethingForever, 2)
   │   └─► computeSomethingForever(3) - runs forever
```

### Runtime Interrupt Flow

```
Application Thread Running
    │
    │ (Timer counting down in background)
    │
    ▼
Timer reaches 0 → IRQ Signal → CPU
    │
    ├─ [Hardware]: Save PC, CPSR to IRQ mode registers
    ├─ [Hardware]: Switch to IRQ mode
    ├─ [Hardware]: Disable further IRQs (CPSR I-bit set)
    ├─ [Hardware]: Set PC from vector table (0x0018)
    │
    ▼
interrupt_vector() Entry (rpi-interrupts.c:93)
    │
    ├─ [Compiler prologue]: Save working registers
    │
    ├─ Check: RPI_GetArmTimer()->MaskedIRQ != 0?
    │   │
    │   └─ YES → This is an ARM Timer interrupt
    │       │
    │       ├─ RPI_GetArmTimer()->IRQClear = 1
    │       │   └─ Clears RAWIRQ flag in timer
    │       │
    │       └─ ticks++
    │           └─ Global variable incremented
    │
    ├─ [Compiler epilogue]: Restore registers
    ├─ [Hardware]: Restore CPSR (re-enables IRQs)
    └─ [Hardware]: Restore PC (return to interrupted code)
    │
    ▼
Application Thread Resumes
```

## Important Components

### 1. Interrupt Vector Table (`startup.c`)

Located at memory address `0x0000`, this table contains addresses of exception handlers:

```c
Entry:
    ldr pc, _reset_h                        // 0x0000: Reset
    ldr pc, _undefined_instruction_vector_h // 0x0004: Undefined Instruction
    ldr pc, _software_interrupt_vector_h    // 0x0008: SWI
    ldr pc, _prefetch_abort_vector_h        // 0x000C: Prefetch Abort
    ldr pc, _data_abort_vector_h            // 0x0010: Data Abort
    ldr pc, _unused_handler_h               // 0x0014: Unused
    ldr pc, _interrupt_vector_h             // 0x0018: IRQ ← Timer interrupts
    ldr pc, _fast_interrupt_vector_h        // 0x001C: FIQ
```

### 2. Timer Configuration (`a4p1.c`)

```c
void initTimerInterrupts()
{
    RPI_EnableARMTimerInterrupt();
    
    // Timer frequency = Clk/256 * 0x400
    // 0xF3C is about 1 second
    RPI_GetArmTimer()->Load = 0xF3C;
    
    // Setup the ARM Timer Control Register
    RPI_GetArmTimer()->Control =
        RPI_ARMTIMER_CTRL_23BIT |       // Use 23-bit counter
        RPI_ARMTIMER_CTRL_ENABLE |      // Enable timer
        RPI_ARMTIMER_CTRL_INT_ENABLE |  // Enable interrupts
        RPI_ARMTIMER_CTRL_PRESCALE_256; // Divide clock by 256
    
    // Enable interrupts globally
    ENABLE();  // Executes: cpsie i
}
```

### 3. Interrupt Handler (`rpi-interrupts.c`)

```c
volatile int ticks = -1;  // Global tick counter

void __attribute__((interrupt("IRQ"))) interrupt_vector(void)
{
    if (RPI_GetArmTimer()->MaskedIRQ) {
        // Clear the ARM Timer interrupt flag
        RPI_GetArmTimer()->IRQClear = 1;
        
        // Increment tick counter
        ticks++;
    }
}
```

The `__attribute__((interrupt("IRQ")))` tells the compiler to:
- Generate special function prologue to save registers
- Generate special epilogue to restore registers
- Return using the appropriate interrupt return instruction

### 4. Application Threads (`a4p1.c`)

```c
void computeSomethingForever(int seg) {
    ExpStruct* value;
    for(volatile uint32_t i=0; ; i++)
    {
        // Compute exponential of integers 1-9
        value = iexp((i%8)+1);
        
        // Display result on PiFace at segment position
        print_at_seg(seg % 4, value->expInt);
    }
}
```

Each thread runs independently, and the global `ticks` variable (updated by interrupts) can be used for timing.

## Memory Layout

```
0x0000_0000: Interrupt Vector Table (32 bytes)
             ├─ 8 vectors × 4 bytes each
             └─ Copied from 0x8000 at startup

0x0000_7000: IRQ Mode Stack (grows down)
             └─ Used during interrupt handling

0x0000_8000: Application Code & Data
             ├─ Program loaded here by bootloader
             ├─ Supervisor Mode Stack (grows down from 0x8000)
             └─ Heap grows up from end of program

Peripherals:
  - ARM Timer: PERIPHERAL_BASE + 0xB400
  - IRQ Controller: RPI_INTERRUPT_CONTROLLER_BASE
  - GPIO: PERIPHERAL_BASE + 0x200000
```

## How Timer Interrupts Work

1. **Timer Setup**: The ARM Timer is configured with a load value and prescaler
2. **Countdown**: Timer counts down from the load value at a rate determined by the prescaler
3. **Interrupt Generation**: When counter reaches 0:
   - Timer reloads the value from the Load register
   - Sets the interrupt pending flag
   - If interrupts are enabled, signals the interrupt controller
4. **Interrupt Routing**: The interrupt controller routes the interrupt to the CPU's IRQ line
5. **CPU Response**: The CPU:
   - Saves the current program counter and status register
   - Switches to IRQ mode
   - Jumps to the IRQ vector (address 0x0018)
   - Executes `interrupt_vector()`
6. **Interrupt Handling**: The ISR:
   - Identifies the interrupt source (ARM Timer)
   - Clears the interrupt flag
   - Performs the necessary action (increment ticks)
   - Returns
7. **Resumption**: CPU restores state and continues where it left off

## References

- [BCM2835 ARM Peripherals](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf)
- [ARM Architecture Reference Manual](https://developer.arm.com/documentation/)
- [Valvers Raspberry Pi Bare Metal Tutorial](https://www.valvers.com/rpi/bare-metal/)

---

**Assignment Context**: DT8025 Real-Time Systems, Assignment 4 Part 1



