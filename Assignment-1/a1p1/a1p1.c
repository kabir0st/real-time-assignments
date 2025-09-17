/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Copyright (c) 2017, Sebastian Kunze <sebastian.kunze@hh.se>
    All rights reserved.
	Wagner de Morais (Wagner.deMorais@hh.se)
*/

#include <stdio.h>
#include <string.h>
#include "uart.h"
#include "iregister.h"

#define LINE 80

int main()
{
	iRegister r;
	char str[LINE];
	char name[LINE];
	char c;
	int inumber, inibble, ibit, ishift = 0;
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

	// Display name length
	uart_puts("Your name has ");
	// Convert length to string for display
	char length_str[10];
	sprintf(length_str, "%d", i);
	uart_puts(length_str);
	uart_puts(" characters.\n");

	// To Display a string
	// uart_puts("String\n");

	// To get one character
	// c=uart_getc();

	// However, to get a number, you need to call uart_getc
	// multiple times until receiving a new line.
	// The results of each call to uart_getc can be stored into str
	// atoi(str) will result a number.

	return 0;
}
