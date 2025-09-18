# Library Documentation

## Overview

This document provides comprehensive documentation for all library components used in the Assignment 1 projects. The libraries provide hardware abstraction and core functionality for bare-metal Raspberry Pi 3 development.

## Core Libraries

### iregister Library (a1p1 only)

#### Purpose
Provides a complete set of bit manipulation operations on a 32-bit integer register, designed for educational purposes to demonstrate low-level bit operations.

#### Header File: `lib/iregister.h`

**Data Structure**:
```c
typedef struct {
    int content;  // 32-bit integer representing the register
} iRegister;
```

**Function Prototypes** (with full documentation in header):

##### Bit Operations
- `void resetBit(int i, iRegister *r)` - Reset specific bit to 0
- `void setBit(int i, iRegister *r)` - Set specific bit to 1
- `int getBit(int i, iRegister *r)` - Read specific bit value
- `void resetAll(iRegister *r)` - Reset all bits to 0
- `void setAll(iRegister *r)` - Set all bits to 1

##### Nibble Operations
- `void assignNibble(int nibble, int pos, iRegister *r)` - Set nibble value
- `int getNibble(int pos, iRegister *r)` - Get nibble value

##### Shift Operations
- `void shiftLeft(int n, iRegister *r)` - Logical left shift
- `void shiftRight(int n, iRegister *r)` - Logical right shift

##### Display Function
- `char *reg2str(iRegister r)` - Convert to binary string representation

#### Implementation File: `lib/iregister.c`

**Key Implementation Details**:

##### Error Handling Pattern
All functions follow consistent error checking:
```c
// Pre-condition checks
if (r == NULL) {
    fprintf(stderr, "Error: A NULL pointer was given to function\n");
    return;
}
if (i < 0 || i > 31) {
    fprintf(stderr, "Error: Invalid bit position\n");
    return;
}

// Operation implementation
r->content &= ~(1 << i);  // Example: reset bit

// Post-condition verification
if ((r->content & (1 << i)) != 0) {
    fprintf(stderr, "Error: Operation failed\n");
    return;
}
```

##### Bit Manipulation Techniques
- **Set Bit**: `r->content |= (1 << i)` - OR with bit mask
- **Reset Bit**: `r->content &= ~(1 << i)` - AND with inverted mask
- **Get Bit**: `(r->content & (1 << i)) ? 1 : 0` - AND with mask and test
- **Nibble Access**: Uses masking and shifting for 4-bit groups

##### String Representation
```c
char* reg2str(iRegister r) {
    static char str[33];  // 32 bits + null terminator
    for (int i = 31; i >= 0; i--) {
        str[31 - i] = (r.content >> i) & 1u ? '1' : '0';
    }
    str[32] = '\0';
    return str;
}
```

### UART Library (a1p1 only)

#### Purpose
Provides UART communication functionality for interactive user interfaces on Raspberry Pi 3.

#### Header File: `lib/uart.h`

**Function Prototypes**:
- `void uart_init()` - Initialize UART peripheral
- `void uart_send(unsigned int c)` - Send single character
- `char uart_getc()` - Receive single character
- `void uart_puts(char *s)` - Send null-terminated string
- `void uart_clear()` - Clear terminal screen
- `void print2uart(const char* fmt, ...)` - Printf-style formatted output

#### Implementation File: `lib/uart.c`

**Hardware Configuration**:
```c
// Auxiliary mini UART registers
#define AUX_ENABLE      ((volatile unsigned int*)(PERIPHERAL_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(PERIPHERAL_BASE+0x00215040))
#define AUX_MU_LCR      ((volatile unsigned int*)(PERIPHERAL_BASE+0x0021504C))
// ... additional register definitions
```

**Initialization Process**:
```c
void uart_init() {
    *AUX_ENABLE |= 1;        // Enable UART1, AUX mini uart
    *AUX_MU_LCR = 3;         // 8 bits
    *AUX_MU_BAUD = 270;      // 115200 baud
    // GPIO pin mapping for UART1
    // Enable Tx, Rx
}
```

**Character Transmission**:
```c
void uart_send(unsigned int c) {
    // Wait until transmitter is ready
    do { asm volatile("nop"); } while(!(*AUX_MU_LSR & 0x20));
    *AUX_MU_IO = c;  // Write character to buffer
}
```

**String Transmission with Line Ending Conversion**:
```c
void uart_puts(char *s) {
    while(*s) {
        if(*s == '\n')
            uart_send('\r');  // Convert LF to CRLF
        uart_send(*s++);
    }
}
```

### LED Library (a1p2 only)

#### Purpose
Provides LED control functionality for GPIO-based LED manipulation on Raspberry Pi 3.

#### Header File: `lib/led.h`

**GPIO Pin Definitions**:
```c
#define LEDHH_GPFSEL      GPFSEL1    // Function select register
#define LEDHH_GPFBIT      18         // Bit position for GPIO16
#define LEDHH_GPSET       GPSET0     // Set register
#define LEDHH_GPCLR       GPCLR0     // Clear register
#define LEDHH_GPIO_BIT    16         // GPIO bit position
#define LEDHH_GPIO        16         // GPIO pin number
```

**Control Macros**:
```c
#define LEDHH_ON()  do { RPI_GetGpio()->LEDHH_GPSET = (1 << LEDHH_GPIO_BIT); } while(0)
#define LEDHH_OFF() do { RPI_GetGpio()->LEDHH_GPCLR = (1 << LEDHH_GPIO_BIT); } while(0)
```

**Function Prototypes**:
- `void led_init()` - Initialize GPIO pin for LED control
- `void led_on()` - Turn LED on
- `void led_off()` - Turn LED off
- `void led_toggle()` - Toggle LED state
- `void led_blink()` - Continuous blinking loop

#### Implementation File: `lib/led.c`

**GPIO Initialization**:
```c
void led_init() {
    // Set GPIO16 as output
    GPIO->GPFSEL1 |= (1 << 18);

#if defined(RPI3) && defined(IOBPLUS)
    // Platform-specific initialization for RPI3B+
    RPI_SetGpioPinFunction(LED_GPIO, FS_OUTPUT);
#endif
}
```

**LED Control Implementation**:
```c
void led_toggle() {
    if (GPIO->GPLEV0 & (1 << 16)) {
        led_off();  // Currently on, turn off
    } else {
        led_on();   // Currently off, turn on
    }
}

void led_blink() {
    while(1) {
        led_toggle();
        RPI_WaitMicroSeconds(1000000);  // 1 second delay
    }
}
```

## Hardware Abstraction Libraries

### GPIO Library

#### Header File: `lib/rpi-gpio.h`

**Purpose**: Provides comprehensive GPIO peripheral control for Raspberry Pi.

**Key Features**:
- Platform-specific LED pin definitions for different RPi models
- GPIO function selection enumeration
- Complete GPIO register structure definition
- GPIO control function prototypes

**Platform Support**:
```c
#if defined(RPI1) && !defined(IOBPLUS)
    #define LED_GPIO    16
#elif defined(RPI3) && defined(IOBPLUS)
    #define LED_GPIO    29
#elif defined(RPI4)
    #define LED_GPIO    42
#endif
```

**GPIO Register Structure**:
```c
typedef struct {
    rpi_reg_rw_t    GPFSEL0;     // Function Select 0
    rpi_reg_rw_t    GPFSEL1;     // Function Select 1
    // ... complete register set
    rpi_reg_wo_t    GPSET0;      // Pin Output Set 0
    rpi_reg_wo_t    GPCLR0;      // Pin Output Clear 0
    rpi_reg_wo_t    GPLEV0;      // Pin Level 0
    // ... additional registers
} rpi_gpio_t;
```

**Function Prototypes**:
- `rpi_gpio_t* RPI_GetGpio(void)` - Get GPIO base pointer
- `void RPI_SetGpioPinFunction(rpi_gpio_pin_t gpio, rpi_gpio_alt_function_t func)`
- `void RPI_SetGpioOutput(rpi_gpio_pin_t gpio)`
- `rpi_gpio_value_t RPI_GetGpioValue(rpi_gpio_pin_t gpio)`

### Base Hardware Definitions

#### Header File: `lib/rpi-base.h`

**Purpose**: Provides fundamental hardware definitions and base addresses.

**Platform Base Addresses**:
```c
#if defined(RPI0) || defined(RPI1)
    #define PERIPHERAL_BASE    0x20000000UL
#elif defined(RPI2) || defined(RPI3)
    #define PERIPHERAL_BASE    0x3F000000UL
#elif defined(RPI4)
    #define PERIPHERAL_BASE    0xFE000000UL
#endif
```

**System Frequencies**:
```c
#if defined(RPI0) || defined(RPI3)
    #define SYSFREQ 400000000UL
#elif defined(RPI1) || defined(RPI2)
    #define SYSFREQ 250000000UL
#elif defined(RPI4)
    #define SYSFREQ 500000000UL
#endif
```

**Register Type Definitions**:
```c
typedef volatile uint32_t rpi_reg_rw_t;        // Read-Write
typedef volatile const uint32_t rpi_reg_ro_t;  // Read-Only
typedef volatile uint32_t rpi_reg_wo_t;        // Write-Only
```

## System Libraries

### System Timer Library

**Files**: `lib/rpi-systimer.h`, `lib/rpi-systimer.c`

**Purpose**: Provides precise timing functions using hardware system timer.

**Key Functions**:
- `void RPI_WaitMicroSeconds(uint32_t us)` - Microsecond delay
- Timer register access for precise timing

### Startup and System Calls

#### Startup Library (`lib/startup.c`)
**Purpose**: Provides boot sequence and initialization for bare-metal applications.

**Key Responsibilities**:
- Initial CPU configuration
- Memory initialization
- Vector table setup
- Jump to main function

#### System Calls Library (`lib/syscalls.c`)
**Purpose**: Provides minimal system call implementations for C standard library support.

**Typical Functions**:
- `_write()` - Output redirection
- `_read()` - Input handling
- Memory allocation stubs
- File system operation stubs

## Memory Management and Safety

### Static Allocation
Most libraries use static allocation to avoid heap management:
```c
static char str[33];  // Static buffer for string representation
```

### Pointer Validation
Consistent NULL pointer checking across all functions:
```c
if (r == NULL) {
    fprintf(stderr, "Error: NULL pointer\n");
    return;
}
```

### Hardware Register Access
Direct memory-mapped I/O for performance:
```c
#define AUX_MU_IO ((volatile unsigned int*)(PERIPHERAL_BASE+0x00215040))
*AUX_MU_IO = character;  // Direct register write
```

## Error Handling Strategy

### Pre-condition Validation
- NULL pointer checks
- Parameter range validation
- Hardware state verification

### Post-condition Verification
- Operation result checking
- Hardware register state validation
- Error reporting to stderr

### Error Reporting
- Descriptive error messages
- Function-specific error context
- stderr output for debugging

## Platform Abstraction

### Conditional Compilation
```c
#if defined(RPI3) && defined(IOBPLUS)
    // Platform-specific code
#endif
```

### Hardware Differences
- GPIO pin assignments vary by model
- Peripheral base addresses differ
- Clock frequencies are model-specific

This library architecture provides a clean separation between application logic and hardware details, enabling portable bare-metal development across different Raspberry Pi models.
