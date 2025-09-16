<> is an unequal sign !=



#endif is a preprocessor directive that closes a conditional compilation block. It works together with #ifndef (and #define) to form a complete header guard.


#ifndef lab0_iregister_h    // Start: "If NOT defined"
#define lab0_iregister_h    // Define the macro
// ... all header content goes here ...
#endif                     // End: Close the conditional block


#ifndef stands for "if not defined" - it's a preprocessor directive that checks if a macro is not defined.

so while importing the function does not get defined twice


How it works:
First inclusion:
#ifndef MYHEADER_H → Checks if MYHEADER_H exists (it doesn't)
#define MYHEADER_H → Creates the macro MYHEADER_H
All content is processed normally
#endif → Closes the block
Second inclusion:
#ifndef MYHEADER_H → Checks if MYHEADER_H exists (it does!)
Skips everything until #endif
#endif → Closes the block

Benefits of Header Guards
Prevents Multiple Inclusion: Same header can be included multiple times safely
Avoids Compilation Errors: No duplicate definitions
Faster Compilation: Preprocessor skips already-processed headers
Standard Practice: Expected in all professional C code
Modular Design: Allows complex include dependencies


Alternative: #pragma once
#pragma once

typedef struct {
    int content;
} iRegister;

void resetBit(int, iRegister *);

Modern compilers support a simpler alternative: But #ifndef/#define/#endif is more portable and works with all compilers.

A macro in C programming is a symbolic name that represents a piece of code or a value. It's defined using the #define preprocessor directive and gets replaced by the preprocessor before compilation.

How Macros Work
Preprocessor stage: Before compilation, the preprocessor finds all #define statements
Text replacement: It replaces every occurrence of the macro name with its value
Compilation: The compiler sees the replaced text, not the macro name

Macro	Variable
Text replacement	Memory storage
No memory allocated	Uses memory
Preprocessor handles	Compiler handles
No type checking	Type checking
Faster execution	Slight overhead


Text Replacement (Macros)
How it Works
Preprocessor stage: Text gets replaced before compilation
No memory allocated: The macro name disappears completely
Direct substitution: Like find-and-replace in a text editor

basically it means literal text replacement or function replacement of the Macro,


things to learn
Stack vs Heap

 Function Pointers
Bit Manipulation


Structs and Unions

// Struct - different data types together
typedef struct {
    int x, y;
    char name[20];
} Point;

// Union - same memory location for different types
typedef union {
    int int_val;
    float float_val;
    char bytes[4];
} DataUnion;
Volatile Keyword
Callback Function

Circular Buffers
Interrupt Service Routines (ISR)

 Memory-Mapped I/O
DMA (Direct Memory Access)
Real-Time Programming Patterns


Memory Management: Understand stack vs heap
Pointers: Master pointer arithmetic and dereferencing
Bit Manipulation: Essential for embedded systems
Error Handling: Always check return values
Const Correctness: Use const to prevent accidental modifications
Volatile: Critical for hardware register access
State Machines: Common in embedded systems
Interrupts: Handle hardware events efficiently
Memory-Mapped I/O: Direct hardware control
Real-Time Patterns: Priority-based scheduling



-Wall: Enables all common warning messages (like unused variables, missing return statements)

-Wextra: Enables additional warning messages beyond -Wall

-std=c99: Specifies the C standard to use (C99 standard)

-g: Includes debugging information in the compiled code (useful for debugging with gdb)

gcc -Wall -Wextra -std=c99 -g -c main.c -o main.o
