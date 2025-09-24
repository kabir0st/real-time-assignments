/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Copyright (c) 2017, Sebastian Kunze <sebastian.kunze@hh.se>
    All rights reserved.
	Wagner de Morais (Wagner.deMorais@hh.se)
*/

#include <stdio.h>
#include <stdlib.h>

#include "lib/expstruct.h"
// #include "piface.h"

#define LINE 32

int main()
{
    printf("Hello\n");
    ExpStruct *exp_result;
    for (int i = 0; i <= 20; i++) {
        exp_result = iexp(i);
        printf("x = %d : Exp Aprox: %d.%d\n", i, exp_result->expInt, exp_result->expFraction);
    }
    free(exp_result);
    return 0;
	// char str[LINE];
	// piface_init();
	// piface_clear();

	// piface_puts("DT8025 - A2P2");
	// RPI_WaitMicroSeconds(2000000);
    // piface_clear();

    // ExpStruct* value;

    // value = iexp(10);

    // sprintf(str,"%d: %d.%d", 10, value->expInt, value->expFraction);
	// piface_puts(str);
	// free(value);

	// return 0;

}
