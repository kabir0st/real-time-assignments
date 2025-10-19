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

#include "expstruct.h"

ExpStruct *iexp(int x){
    ExpStruct *e = malloc(sizeof(ExpStruct));
    // pre condition check
    if (x < 0 || x > 20) {
        printf("Input out of range\n");
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
        printf("Post-condition failed: expInt is incorrect\n");
        free(e);
        return NULL;
    }
    if (e->expFraction != frac_two_decimals && e->expFraction >= 0 && e->expFraction < 100) {
        printf("Post-condition failed: expFraction is incorrect\n");
        free(e);
        return NULL;
    }
    return e;
}
