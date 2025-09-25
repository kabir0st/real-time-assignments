/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Copyright (c) 2017, Sebastian Kunze <sebastian.kunze@hh.se>
    All rights reserved.
 */
/*
 * Modified by Wagner Morais on Aug 2022.
 */

#ifndef _EXPSTRUCT_H
#define _EXPSTRUCT_H

struct expState {
    // x is the input
    int x;
    // this is the current term index
    int n;
    // value of e^n-1
    double last_calculated_term;
    double n_sum;
    int completed;
    int n_exp_int;
    int n_exp_fraction;
};

typedef struct expState ExpProgramState;


/** @brief Computes an approximation of e^x and returns integer and fractional parts
 * using expStruct structure defined above.
 *
 * @param x The exponent, an integer in the range 0 to 20 inclusive.
 *
 * @pre  The param x should be greater than equal to 0 and less than or equal to 20
 * @post Returns a valid ExpStruct with:
 *       - expInt equal to integer part of e^x
 *       - expFraction equal to fractional part times 100 (two digits)
 * @return ExpStruct* Pointer to an ExpStruct with integer part in expInt and fractional
 *                    part scaled to two digits in expFraction. NULL if input is invalid
 *                    or memory allocation fails.
 *
 */
ExpProgramState *iexp(ExpProgramState *cur_state);


/**
 * @brief Delays execution for a specified number of milliseconds.
 * @param milliseconds The number of milliseconds to delay.
 *
 * @note This function uses nanosleep for a more accurate and CPU-friendly delay
 *       compared to a busy-wait loop. It also correctly handles interruptions.
 */
void delay_ms(long milliseconds);

#endif
