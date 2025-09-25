/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Copyright (c) 2017, Sebastian Kunze <sebastian.kunze@hh.se>
    All rights reserved.
*/
/*
 * Modified by Wagner Morais on Aug 2023.
*/
#include <stdio.h>
#include <stdlib.h>
#include "expstruct.h"
#include "piface.h"
#include "rpi-systimer.h"

#define LINE 32

int main()
{
    char str[LINE];
	piface_init();
	piface_clear();
    piface_puts("DT8025 - A2P3");
    RPI_WaitMicroSeconds(2000000);
    piface_clear();
    ExpStruct* value;

    // for testing
    while(1){
        for (int i = 0; i < 21; i++) {
            value = iexp(i);
            sprintf(str,"e^%d = %d.%02d\n", i, value->expInt, value->expFraction);
            piface_puts(str);
        }
    }
    free(value);
	return 0;
}
