# TinyThreads x64 - Quick Start Guide

## Build and Run

```bash
cd /home/kabir0st/university/real-time/Assignment-3/x64_codes

# Build the project
make

# Run the program (press Ctrl+C to stop)
./main

# Or build and run in one command
make run
```

## What You'll See

The program demonstrates cooperative multithreading with two threads:
- **Thread T0**: Computes and displays squares (n²)
- **Thread T1**: Finds and displays prime numbers

Both threads cooperatively yield to each other every 0.5 seconds, showing interleaved output like:
```
T1: Prime 2
T0: 0^2=0
T1: Prime 3
T0: 1^2=1
T1: Prime 5
T0: 2^2=4
...
```

## Key Features

✅ **Cooperative Threading**: Threads voluntarily yield control using `yield()`
✅ **Portable**: Uses POSIX `ucontext` API instead of ARM assembly
✅ **Signal-based Interrupts**: Simulates interrupt enable/disable with signal masking
✅ **Standard I/O**: Uses `printf()` and `usleep()` instead of Raspberry Pi hardware

## Architecture

### Threading API
- `spawn(function, arg)` - Create a new thread
- `yield()` - Voluntarily give up the CPU
- Thread queues: `freeQ`, `readyQ`, `doneQ`

### Key Differences from Raspberry Pi Version

| Feature | Raspberry Pi | x64 Linux |
|---------|-------------|-----------|
| Context switching | `setjmp`/`longjmp` | `ucontext` (getcontext/setcontext) |
| Interrupts | ARM CPSIE/CPSID | Signal masking (sigprocmask) |
| Display | PiFace LCD | printf() |
| Timing | RPI_WaitMicroSeconds | usleep() |
| Stack size | 1024 bytes | 8192 bytes |

## Modifying the Code

### Add Your Own Thread

Edit `main.c`:

```c
void myCustomThread(int seg) {
    for(int i = 0; ; i++) {
        printf("T%d: Custom output %d\n", seg, i);
        usleep(500000);  // 0.5 second delay
        yield();         // Give other threads a chance
    }
}

int main() {
    printf("Starting threads...\n");
    spawn(myCustomThread, 2);  // Add your thread
    spawn(computePower, 0);
    computePrimes(1);
    return 0;
}
```

### Change Thread Behavior

- Modify delay: Change `usleep(500000)` value (microseconds)
- Add more threads: Call `spawn()` multiple times (max 5 threads by default)
- Change max threads: Edit `NTHREADS` in `lib/tinythreads.c`

## Troubleshooting

### Program doesn't output anything
- Make sure you compiled successfully: `make clean && make`
- Check for errors in the terminal

### Segmentation fault
- The ucontext-based implementation should be stable
- If issues occur, increase `STACKSIZE` in `lib/tinythreads.c`

### Too many threads
- Default max is 5 threads (change `NTHREADS` constant)
- Remember: `spawn()` creates a thread, main context counts as one

## Clean Up

```bash
# Remove compiled files
make clean

# Rebuild everything
make rebuild
```

## Testing Ideas

1. **Thread Priorities**: Modify the yield frequency in different threads
2. **Resource Sharing**: Add shared variables and mutex locks (Assignment 4)
3. **Performance**: Remove delays and see raw threading performance
4. **Debugging**: Use `printTinyThreadsUART()` to inspect thread queues

## Next Steps (Assignment 4)

The following features are stubbed for future implementation:
- `lock()` / `unlock()` - Mutex support
- `spawnWithDeadline()` - Real-time scheduling
- `scheduler_RR()` - Round-robin scheduling
- `scheduler_RM()` - Rate monotonic scheduling
- `scheduler_EDF()` - Earliest deadline first scheduling

Happy threading! 🧵

