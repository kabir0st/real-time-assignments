/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Copyright (c) 2017, Sebastian Kunze <sebastian.kunze@hh.se>
    All rights reserved.
 */
/*
 * Modified by Wagner Morais on Aug 2022.
 */

#include <stdlib.h>
#include <stdio.h>

#include "expstruct.h"


const int TERM_LIMIT = 100;
const double EPS_FOR_2DP = 0.005;  // conservative for rounding to 2 decimals

ExpProgramState *iexp(ExpProgramState *c_state) {
    // pre condition check
    // if the check logic was long could have
    // skipped using a flag on state.
    if (c_state->x < 0 || c_state->x > 20) {
        printf("Input out of range\n");
        c_state->n_exp_int = 0;
        c_state->n_exp_fraction = 0;
        c_state->completed = 1;
        return c_state;
    }
    if (c_state->x == 0) {
        c_state->n_exp_int = 1;     // e^0 = 1
        c_state->n_exp_fraction = 0;
        c_state->completed = 1;
        return c_state;
    }
    if (c_state->completed) {
        return c_state;  // already computed
    }

    for (int n = c_state->n; n < TERM_LIMIT; ++n) {
        c_state->last_calculated_term *= (double)c_state->x / (double)n;
        c_state->n_sum += c_state->last_calculated_term;
        if (c_state->last_calculated_term < EPS_FOR_2DP) {
            break;
        }
    }
    /* Split into integer + two-decimal fractional parts with rounding. */
    long long scaled = (long long)(c_state->n_sum * 100.0 + 0.5);  // rounded to 2 dp
    int integer_part = (int)(scaled / 100);
    int frac_two_decimals = (int)(scaled % 100);

    /* Handle carry from rounding like 2.999 -> 3.00 */
    if (frac_two_decimals >= 100) {
        integer_part += 1;
        frac_two_decimals = 0;
    }
    c_state->n_exp_int = integer_part;
    c_state->n_exp_fraction = frac_two_decimals;
    c_state->completed = 1;
    // Post-condition: Ensure the structure contains valid values
    if (c_state->n_exp_int != integer_part) {
        printf("Post-condition failed: expInt is incorrect\n");
        return NULL;
    }
    if (c_state->n_exp_fraction != frac_two_decimals && c_state->n_exp_fraction >= 0 && c_state->n_exp_fraction < 100) {
        printf("Post-condition failed: expFraction is incorrect\n");
        return NULL;
    }
    return c_state;
}
