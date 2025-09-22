/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Copyright (c) 2017, Sebastian Kunze <sebastian.kunze@hh.se>
    All rights reserved.
*/
/*
 * Modified by Wagner Morais on Aug 2023.
*/
#include <stdio.h>
#include "expstruct.h"
#include "led.h"

// #define LINE 32

int main()
{
	
	// char str[LINE];
	led_init();
	RPI_WaitMicroSeconds(2000000);	
	
    ExpStruct* value;
  
    int i = 1; 
	// cyclic execution
    while (1) {
		value = iexp(i++);
		// sprintf(str,"%d: %d.%d\n", i, value->expInt, value->expFraction);
		// piface_puts(str);		
		led_toggle();	// or led_blink();
		if (i >= 20)
			i = 1;
		free(value);
		// piface_clear();
    }
	return 0;
}
