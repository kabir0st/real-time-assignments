# A1P2 - LED Control

## Overview

This project implements a simple bare-metal LED control application for Raspberry Pi 3. It demonstrates basic GPIO manipulation by continuously blinking an LED at 1-second intervals.

## Features

- **LED Initialization**: Configures GPIO pin as output for LED control
- **LED Control Functions**: On, off, and toggle operations
- **Continuous Blinking**: Infinite loop with precise timing
- **Multi-Platform Support**: Compatible with different Raspberry Pi models

## File Structure

### Main Application
- **`a1p2.c`** - Minimal main application that initializes and starts LED blinking

### LED Library
- **`lib/led.h`** - LED control interface definitions and macros
- **`lib/led.c`** - LED control implementation with GPIO manipulation

### Hardware Abstraction
- **`lib/rpi-gpio.h`** - GPIO peripheral definitions and control functions
- **`lib/rpi-systimer.h`** - System timer interface for precise delays
- **`lib/rpi3.h`** - Hardware-specific definitions for Raspberry Pi 3

### Build System
- **`Makefile`** - Build configuration for ARM cross-compilation

## Core Functionality

### Main Application (`a1p2.c`)

The main application is intentionally minimal:

```c
int main() {
    led_init();    // Initialize LED GPIO pin
    led_blink();   // Start infinite blinking loop
    return 0;      // Never reached
}
```

### LED Library Functions

#### `led_init()`
- **Purpose**: Initializes GPIO pin for LED control
- **Implementation**:
  - Sets GPIO16 function select register to output mode
  - Conditional compilation for RPI3B+ (GPIO29) support
- **Hardware Access**: Direct register manipulation via `GPIO->GPFSEL1`

#### `led_on()`
- **Purpose**: Turns the LED on
- **Implementation**:
  - Sets GPIO16 output high via `GPIO->GPSET0`
  - Platform-specific handling for different RPi models
- **Note**: LED behavior (active high/low) depends on hardware wiring

#### `led_off()`
- **Purpose**: Turns the LED off
- **Implementation**:
  - Clears GPIO16 output via `GPIO->GPCLR0`
  - Platform-specific handling for different RPi models

#### `led_toggle()`
- **Purpose**: Toggles LED state (on→off or off→on)
- **Implementation**:
  - Reads current GPIO level via `GPIO->GPLEV0`
  - Calls appropriate on/off function based on current state
- **Logic**: Uses bit masking to check GPIO16 level

#### `led_blink()`
- **Purpose**: Continuously blinks LED at 1-second intervals
- **Implementation**:
  - Infinite while loop
  - Calls `led_toggle()` to change state
  - Uses `RPI_WaitMicroSeconds(1000000)` for 1-second delay
- **Timing**: 1,000,000 microseconds = 1 second delay

## Hardware Configuration

### GPIO Pin Assignments

#### Standard Configuration (GPIO16)
```c
#define LEDHH_GPFSEL      GPFSEL1   // Function select register 1
#define LEDHH_GPFBIT      18        // Bit position in GPFSEL1
#define LEDHH_GPSET       GPSET0    // Set register
#define LEDHH_GPCLR       GPCLR0    // Clear register
#define LEDHH_GPIO_BIT    16        // Bit position in set/clear registers
#define LEDHH_GPIO        16        // GPIO pin number
```

#### Platform-Specific Support
The code includes conditional compilation for different Raspberry Pi models:

- **RPI3B+**: Uses dedicated ACT LED on GPIO29
- **Other Models**: Uses GPIO16 configuration

### Register Operations

#### Function Select Configuration
```c
GPIO->GPFSEL1 |= (1 << 18);  // Set GPIO16 as output
```
- Sets bits 18-20 in GPFSEL1 to configure GPIO16 function
- Value `001` in these bits = output function

#### GPIO Set/Clear Operations
```c
// Turn LED on
GPIO->GPSET0 |= (1 << 16);   // Set GPIO16 high

// Turn LED off
GPIO->GPCLR0 |= (1 << 16);   // Clear GPIO16 low
```

#### GPIO Level Reading
```c
if (GPIO->GPLEV0 & (1 << 16)) {
    // GPIO16 is currently high
}
```

## Timing Implementation

The project uses the Raspberry Pi system timer for precise delays:

### `RPI_WaitMicroSeconds(1000000)`
- **Purpose**: Provides accurate microsecond-level delays
- **Parameter**: 1,000,000 microseconds (1 second)
- **Implementation**: Uses hardware system timer for precision
- **Advantage**: More accurate than software loops

## Platform Compatibility

### Conditional Compilation
The code uses preprocessor directives for different Raspberry Pi models:

```c
#if defined( RPI3 ) && defined( IOBPLUS )
    // RPi3B+ specific code using LED_GPIO (GPIO29)
    GPIO->LED_GPSET |= (1 << LED_GPIO_BIT);
#endif
```

### LED Pin Mapping
Different Raspberry Pi models use different GPIO pins for their activity LEDs:
- **RPi1/RPi2**: GPIO47
- **RPi3**: No direct GPIO access (ioexpander)
- **RPi3B+**: GPIO29
- **RPi4**: GPIO42

## Memory and Performance

### Direct Register Access
- Uses direct memory-mapped I/O for maximum performance
- No system calls or library overhead
- Minimal memory footprint

### Infinite Loop Design
- Application runs indefinitely until power off
- No cleanup required in bare-metal environment
- Simple state machine with two states (on/off)

## Build Configuration

The Makefile includes LED-specific object files:

```makefile
OBJS = lib/led.o lib/rpi-gpio.o lib/rpi-armtimer.o
OBJS += lib/rpi-interrupts.o lib/rpi-systimer.o
OBJS += lib/startup.o lib/syscalls.o
OBJS += $(MAINFILE).o
```

### Key Dependencies
- **led.o**: LED control functions
- **rpi-gpio.o**: GPIO hardware abstraction
- **rpi-systimer.o**: System timer for delays
- **startup.o**: Boot and initialization code

## Error Handling

The LED library uses simple, fail-safe approaches:
- Direct hardware register access (no error returns needed)
- Conditional compilation prevents incompatible code execution
- Hardware initialization is straightforward and rarely fails

## Power Considerations

### LED Current Draw
- GPIO pins can source/sink limited current (~16mA typical)
- External current-limiting resistors may be required
- LED forward voltage and current should be within GPIO specifications

### Power Management
- Continuous operation at 1Hz frequency
- Minimal CPU usage (most time spent in delay function)
- No power-saving features implemented (bare-metal application)

## Usage

1. **Build**: Run `make all` to generate `a1p2.img`
2. **Deploy**: Copy image to Raspberry Pi SD card boot partition
3. **Run**: Power on Raspberry Pi - LED should start blinking immediately
4. **Stop**: Power off the device (no graceful shutdown mechanism)

## Troubleshooting

### LED Not Blinking
- Check GPIO pin connections
- Verify correct Raspberry Pi model configuration
- Ensure LED is connected with proper polarity
- Check for sufficient power supply

### Build Issues
- Verify ARM cross-compilation toolchain installation
- Check include paths for library headers
- Ensure all object files are generated correctly
