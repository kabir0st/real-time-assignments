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
#include "lib/expstruct.h"
#include <time.h>

void delay_ms(long milliseconds) {
    struct timespec req, rem;

    if (milliseconds <= 0) return;

    req.tv_sec = milliseconds / 1000;
    req.tv_nsec = (milliseconds % 1000) * 1000000L;

    // Loop until the full delay is completed, handling interruptions
    while (nanosleep(&req, &rem) == -1) {
        req = rem;
    }
}

int main()
{
    ExpStruct* value;
    printf("starting . . . \n");
    // for testing
    for (int i = 0; i < 21; i++) {
        value = iexp(i);
        printf("e^%d = %d.%02d\n", i, value->expInt, value->expFraction);
    }
    free(value);
	return 0;
}
