/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Copyright (c) 2017, Sebastian Kunze <sebastian.kunze@hh.se>
    All rights reserved.
	Wagner de Morais (Wagner.deMorais@hh.se)
*/

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "lib/expstruct.h"


#define LINE 32

/**
 * @brief Delays execution for a specified number of milliseconds.
 * @param milliseconds The number of milliseconds to delay.
 *
 * @note This function uses nanosleep for a more accurate and CPU-friendly delay
 *       compared to a busy-wait loop. It also correctly handles interruptions.
 */
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
    printf("Hello\n");
    ExpStruct *exp_result;
    double actual = 1;
    for (int i = 0; i <= 20; i++) {

        exp_result = iexp(i);
        actual = (exp(i));
        printf("x = %d : Exp Aprox: %d.%02d  : Actual: %.2f\n", i, exp_result->expInt, exp_result->expFraction, actual);

        // Introduce a 500ms delay
        delay_ms(1000);
    }
    free(exp_result);
    return 0;
}
