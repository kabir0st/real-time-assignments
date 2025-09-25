/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Copyright (c) 2017, Sebastian Kunze <sebastian.kunze@hh.se>
    All rights reserved.
	Wagner de Morais (Wagner.deMorais@hh.se)
*/

#include <stdio.h>

#include "expstruct.h"
#include "piface.h"
#include <stdlib.h>
#include "rpi-systimer.h"

#define LINE 32

int main()
{
	
	char str[LINE];
	piface_init();

	while(1)
	{

		piface_clear();
		
		piface_puts("DT8025 - A2P2");
		RPI_WaitMicroSeconds(2000000);	
		piface_clear();

		for (int n = 1; n <= 20; ++n) {
			ExpStruct* value = iexp(n);
			if (!value) {
				piface_puts("iexp() failed");
				break;
			}

			// Print "n: int.frac" with two digits in the fraction (e.g., 2.05)
			// Note the %02d to zero-pad the fractional part.
			snprintf(str, LINE, "%d: %d.%02d", n, value->expInt, value->expFraction);

			piface_clear();
			piface_puts(str);

			free(value);

			// slow down so each value is visible on the LCD
			RPI_WaitMicroSeconds(800000); 
		}

		piface_clear();
		piface_puts("Finito");
		RPI_WaitMicroSeconds(800000);

	}

    return 0;
    
    //value = iexp(10);
	
    //sprintf(str,"%d: %d.%d", 10, value->expInt, value->expFraction);
	//piface_puts(str);
	//free(value);

	//return 0;

}