## notes

Purpose of PRIMASK
Change how the CPU responds to interrupts.
Normal Operation (PRIMASK = 0, interrupts enabled):
CPU executes your code
When an interrupt occurs (e.g., timer, I/O event):
CPU pauses current execution
Saves context (registers) to stack
Jumps to interrupt handler routine
After handler completes, restores context and resumes
With PRIMASK Set (PRIMASK = 1, interrupts disabled):
CPU executes your code continuously
When an interrupt occurs:
Interrupt is marked as "pending"
CPU ignores it and keeps executing current code
No context switch happens
The interrupt stays pending until PRIMASK is cleared

Why this matters for tinythreads:
Without DISABLE(), a timer interrupt could trigger a context switch in the middle of queue manipulation
This could corrupt the linked list structure (e.g., half-updated pointers)
DISABLE() ensures these critical sections run atomically without interruption

DISABLE();              // Set PRIMASK = 1
// ... manipulate thread queues ...
// ... setup new thread context ...
enqueue(newp, &readyQ); // Critical operations
ENABLE();               // Clear PRIMASK = 0
// Now pending interrupts can be serviced

Exceptions: Even with PRIMASK set, NMI (Non-Maskable Interrupt) and HardFault can still interrupt - these are for critical system failures that must always be handled.

# Questions


1. What is the purpose of the `spawn` function? How does it work?





2. What is the purpose of the `dispatch` function? How does it work?
3. What is the purpose of the `yield` function? How does it work?
4. When is the code `current->function(current->arg);` within the `spawn` function
executed?
5. After finishing the execution of line 3, describe the content of `readyQ`.
i. Note: you might want to draw a diagram to visualize the queue.
6. After finishing the execution of line 3, describe the content of `freeQ`.
• Note: you might want to draw a diagram to visualize the queue.
7. Which task, i.e., computePower and computePrimes, executes first, and why?
8. Although functions `computePower` and `computePrimes` never return, they execute
concurrently in the `a3p1.img` kernel. How is this achieved?
9. In a hypothetical scenario where `computePower` and `computePrimes` do return, which implies
that the threads assigned to execute the tasks will terminate, you may need to track which threads
terminated. One approach is to keep information about the terminated threads in a list called
`doneQ`. Where in `lib/tinythreads.c` would you add a thread that terminated into `doneQ`?
