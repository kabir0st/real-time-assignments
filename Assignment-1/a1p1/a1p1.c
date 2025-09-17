/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Copyright (c) 2017, Sebastian Kunze <sebastian.kunze@hh.se>
    All rights reserved.
	Wagner de Morais (Wagner.deMorais@hh.se)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lib/uart.h"
#include "lib/iregister.h"

#define LINE 80

// Helper function to read a number using UART
int uart_read_number() {
    char str[LINE];
    char c;
    int i = 0;

    while (i < LINE - 1) {
        c = uart_getc();
        if (c == '\n' || c == '\r') {
            break;
        }
        str[i] = c;
        i++;
    }
    str[i] = '\0';
    return atoi(str);
}

int main()
{
	iRegister r;
	char name[LINE];
	char c;
	int initial_value, bit_pos, nibble_val, shift_val;
	int i = 0;

	// Using the uart
	// First, initialize and clear the channel
	uart_init();
	uart_clear();

	// Prompt user for their name
	uart_puts("Enter your name: ");

	// Read name character by character until newline
	i = 0;
	while (i < LINE - 1) {
		c = uart_getc();

		// Check for newline or carriage return to end input
		if (c == '\n' || c == '\r') {
			break;
		}

		// Store character in name array
		name[i] = c;
		i++;
	}

	// Null terminate the string
	name[i] = '\0';

	// Display the entered name
	uart_puts("Hello, ");
	uart_puts(name);
	uart_puts("! Welcome to the Real-Time Embedded Systems course.\n");
	uart_puts("--------------------------------\n");

	// Get initial value for register
	uart_puts("Enter initial value (decimal): ");
	initial_value = uart_read_number();
	r.content = initial_value;

	uart_puts("Initial Value: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);
	uart_puts("--------------------------------\n");

	// Test resetBit
	uart_puts("Enter bit position to reset (0-31): ");
	bit_pos = uart_read_number();

	uart_puts("Before reset: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);

	resetBit(bit_pos, &r);

	uart_puts("After reset: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);
	uart_puts("--------------------------------\n");

	// Test resetAll
	r.content = initial_value;
	uart_puts("Testing resetAll - Before: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);

	resetAll(&r);

	uart_puts("After resetAll: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);
	uart_puts("--------------------------------\n");

	// Test setBit
	r.content = initial_value;
	uart_puts("Enter bit position to set (0-31): ");
	bit_pos = uart_read_number();

	uart_puts("Before set: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);

	setBit(bit_pos, &r);

	uart_puts("After set: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);
	uart_puts("--------------------------------\n");

	// Test setAll
	r.content = 0;
	uart_puts("Testing setAll - Before: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);

	setAll(&r);

	uart_puts("After setAll: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);
	uart_puts("--------------------------------\n");

	// Test getBit
	r.content = initial_value;
	uart_puts("Enter bit position to read (0-31): ");
	bit_pos = uart_read_number();

	uart_puts("Register: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);
	print2uart("Bit at position %d: %d\n", bit_pos, getBit(bit_pos, &r));
	uart_puts("--------------------------------\n");

	// Test nibble operations
	r.content = initial_value;
	uart_puts("Testing nibble operations:\n");
	uart_puts("Register: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);
	print2uart("Lower nibble (pos 1): %d\n", getNibble(1, &r));
	print2uart("Upper nibble (pos 2): %d\n", getNibble(2, &r));

	uart_puts("Enter value for lower nibble (0-15): ");
	nibble_val = uart_read_number();
	assignNibble(nibble_val, 1, &r);
	uart_puts("After setting lower nibble: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);

	uart_puts("Enter value for upper nibble (0-15): ");
	nibble_val = uart_read_number();
	assignNibble(nibble_val, 2, &r);
	uart_puts("After setting upper nibble: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);
	uart_puts("--------------------------------\n");

	// Test shift operations
	r.content = initial_value;
	uart_puts("Testing shift operations:\n");
	uart_puts("Initial: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);

	uart_puts("Enter positions to shift left: ");
	shift_val = uart_read_number();
	shiftLeft(shift_val, &r);
	uart_puts("After left shift: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);

	uart_puts("Enter positions to shift right: ");
	shift_val = uart_read_number();
	shiftRight(shift_val, &r);
	uart_puts("After right shift: ");
	uart_puts(reg2str(r));
	print2uart(" = %d\n", r.content);
	uart_puts("--------------------------------\n");

	uart_puts("All operations completed!\n");

	return 0;
}
