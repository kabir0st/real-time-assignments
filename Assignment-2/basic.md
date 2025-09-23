# PiFace Control and Display Driver (`piface.c`)

## Overview

This file implements a bare-metal driver for the PiFace Control and Display board, designed for the Raspberry Pi. It provides SPI communication routines, MCP23S17 I/O expander control, and HD44780-compatible LCD display functions. The code is intended for educational use in real-time embedded systems courses.

- SPI: Serial Peripheral Interface is a de facto standard for synchronous serial communication, used primarily in embedded systems for short-distance wired communication between integrated circuits.


## Hardware References

- **PiFace Control and Display Board**: [Official PiFace Documentation](http://piface.github.io/libpifacedigital/)
- **MCP23S17 I/O Expander**: [Microchip MCP23S17 Datasheet (PDF)](https://ww1.microchip.com/downloads/en/devicedoc/20001952c.pdf)
- **HD44780 LCD Controller**: [Hitachi HD44780U LCD Controller Datasheet (PDF)](https://www.sparkfun.com/datasheets/LCD/HD44780.pdf)
- **Raspberry Pi GPIO**: [BCM2835 ARM Peripherals (PDF)](https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/BCM2835-ARM-Peripherals.pdf)

## Main Components

### 1. SPI Bit-Banging

The code implements SPI communication using GPIO pins, manually toggling the clock, data, and chip enable lines.

- **spi_init**: Configures GPIO pins for SPI (CE, CLK, MOSI, MISO).
- **spi_start/spi_stop**: Controls the chip enable (CE) line.
- **spi_byte**: Sends and receives a byte over SPI by toggling MOSI, CLK, and reading MISO.

### 2. MCP23S17 I/O Expander

The MCP23S17 is a SPI-based GPIO expander. The code provides:

- **mcp_read**: Reads a register from MCP23S17.
- **mcp_write**: Writes a value to a register.
- **mcp_init**: Initializes Port A (inputs with pull-ups) and Port B (outputs for LCD).

### 3. LCD Control (HD44780)

The LCD is controlled in 4-bit mode via MCP23S17 Port B.

- **lcd_read_busy_flag_register**: Reads the busy flag from the LCD to synchronize commands.
- **lcd_busy_wait**: Waits until the LCD is ready.
- **lcd_pulse**: Generates an enable pulse for the LCD.
- **lcd_write_cmd/lcd_write_data**: Sends commands and data to the LCD in two 4-bit nibbles.
- **lcd_init**: Initializes the LCD for 4-bit operation, sets display parameters.

### 4. Display Functions

- **piface_init**: Initializes SPI, MCP23S17, and LCD.
- **piface_putc/piface_puts**: Writes characters and strings to the LCD, handling line wrapping and newlines.
- **piface_clear**: Clears the LCD display.
- **piface_set_cursor**: Placeholder for setting cursor position (to be implemented).
- **print_at_seg/printf_at_seg**: Placeholders for segmented display output (to be implemented).

## Code Flow

1. **Initialization**:
   Call `piface_init()` to set up SPI, MCP23S17, and LCD.

2. **Writing Data**:
   Use `piface_putc(char)` or `piface_puts(char[])` to display text. The code manages cursor position and line wrapping.

3. **Clearing Display**:
   Call `piface_clear()` to reset the LCD.

4. **Advanced Features**:
   Functions for cursor positioning and segmented display output are provided as stubs for further development.

## Key Implementation Details

- **Bit-banging SPI**: Direct manipulation of GPIO registers for SPI protocol.
- **LCD 4-bit Mode**: Data sent in two nibbles, with enable pulses and busy flag synchronization.
- **Display Buffering**: The `cnt` variable tracks the current character position for line management.

## References

- [PiFace Control and Display Board Documentation](http://piface.github.io/libpifacedigital/)
- [MCP23S17 Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/20001952c.pdf)
- [HD44780 LCD Controller Datasheet](https://www.sparkfun.com/datasheets/LCD/HD44780.pdf)
- [Raspberry Pi BCM2835 ARM Peripherals](https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/BCM2835-ARM-Peripherals.pdf)

## Further Reading

- [libpifacedigital Source Code](https://github.com/piface/libpifacedigital)
- [Embedded C for ARM Cortex-M](https://www.arm.com/resources/education/education-kit/embedded-cortex-m)

---
For function references, see [`piface_init`](Assignment-2/a2p1/lib/piface.c), [`piface_putc`](Assignment-2/a2p1/lib/piface.c), [`piface_puts`](Assignment-2/a2p1/lib/piface.c), and [`lcd_write_cmd`](Assignment-2/a2p1/lib/piface.c).
