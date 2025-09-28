/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Copyright (c) 2017, Sebastian Kunze <sebastian.kunze@hh.se>
    All rights reserved.
 */
/*
 * Modified by Wagner Morais on Aug 2022.
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "led.h"
#include "expstruct.h"
#include "rpi-systimer.h"

ExpStruct *iexp(int x){
	ExpStruct *e = malloc(sizeof(ExpStruct));
    extern int total_iterations;

    // pre condition check
    if (x < 0 || x > 20) {
        e->expInt = 0;
        e->expFraction = 0;
        return e;
    }
    if (x == 0) {
        e->expInt = 1;     // e^0 = 1
        e->expFraction = 0;
        return e;
    }

    const int TERM_LIMIT = 100;
    const double EPS_FOR_2DP = 0.005;  // conservative for rounding to 2 decimals

    double sum = 1.0;   // k = 0 term
    double term = 1.0;  // current term (starts at x^0/0! = 1)

    for (int k = 1; k < TERM_LIMIT; ++k) {
        term *= (double)x / (double)k;  // term_k = term_{k-1} * x/k
        sum += term;
		total_iterations++;
        // this is not a ideal approach as this intentionally
        // limits the performance of the function
        // but it's just to show the values on the piface
        // after the calculation of e^x.

        // we can use a cached array to store the values
        // of e^x for x = 0 to 20 to avoid recalculating
        // them again, but the assignment does not
        // specify that we can do that, so we are
        // calculating them every time.

        // and we can use the same cached array to show the values
        // on the piface and handle the clearing of the display
        // separately so we won't have to use the delay, but I am
        // not sure if we are supposed to do that so eh.

        if (total_iterations > 200){
            // toggling every 100 total iteration used
            // by all calls to iexp
            led_toggle();
            // reseting iteration after toggling
            total_iterations = 0;
        }
        if (term < EPS_FOR_2DP) {
            break;
        }
    }

    /* Split into integer + two-decimal fractional parts with rounding. */
    long long scaled = (long long)(sum * 100.0 + 0.5);  // rounded to 2 dp
    int integer_part = (int)(scaled / 100);
    int frac_two_decimals = (int)(scaled % 100);

    /* Handle carry from rounding like 2.999 -> 3.00 */
    if (frac_two_decimals >= 100) {
        integer_part += 1;
        frac_two_decimals = 0;
    }
    e->expInt = integer_part;
    e->expFraction = frac_two_decimals;
    // Post-condition: Ensure the structure contains valid values
    if (e->expInt != integer_part) {
        free(e);
        return NULL;
    }
    if (e->expFraction != frac_two_decimals && e->expFraction >= 0 && e->expFraction < 100) {
        free(e);
        return NULL;
    }
    return e;
}
