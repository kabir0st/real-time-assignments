/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Copyright (c) 2017, Sebastian Kunze <sebastian.kunze@hh.se>
    All rights reserved.
	Wagner de Morais (Wagner.deMorais@hh.se)
*/

#include <stdio.h>
#include "uart.h"
#include "iregister.h"

#define LINE 80

int main()
{
	iRegister r;
	char str[LINE];
	char c;
	int inumber, inibble, ibit, ishift = 0;
	// Using the uart
	// First, initialize and clear the channel
	// uart_init();
	// uart_clear();
	
	// To Display a string
	// uart_puts("String\n");
	
	// To get one character
	// c=uart_getc();
	
	// However, to get a number, you need to call uart_getc 
	// multiple times until receiving a new line.
	// The results of each call to uart_getc can be stored into str
	// atoi(str) will result a number.
}