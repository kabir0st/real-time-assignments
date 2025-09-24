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

#define CACHE_SIZE 21  // 0 to 20

// so In C global or static arrays are
//  zero-initialized automatically
// we define it as this with a memory size
// cause we know the input will me limited to
// just 20, so 21 values.
double factorial_cache[CACHE_SIZE];

double exp_cache[CACHE_SIZE];

// it's in single thread context so no need to
// lock the value to be thread safe and to
// avoid race conditions, for now

double calculate_factorial(int x){
    if (x < 0) {
        printf("Input out of range\n");
        return 0;
    } // x must be +ve
    if (x == 0) return 1; // base case
    if (x < CACHE_SIZE && factorial_cache[x] != 0) return factorial_cache[x]; // cached value
    double result = x * calculate_factorial(x - 1); // recursive case
    if (x < CACHE_SIZE) factorial_cache[x] = result; // store in cache
    return result;
}

// implemented power function to avoid
// using math.h library
double power(int base, int exponent) {
    double result = 1;
    for (int i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}


double calculate_exponential(int x){
    if (exp_cache[x] != 0) {
        return exp_cache[x]; // return cached value
    }
    int term = 0; // first term is always 1
    double sum = 0;
    double dx = 0;
    while (term < 100){
        dx = power(x, term) / calculate_factorial(term);
        if (dx < 0.01 ) // stop if less than 2 decimal points
        {
            // printf("Decimal precision reached\n");
            break;
        }
        sum += dx;
        term++;
    }
    return sum;
}

ExpStruct *iexp(int x){
    if (x <= 0 || x >= 20) {
        printf("Input out of range\n");
        return NULL;
    }
    ExpStruct *e = malloc(sizeof(ExpStruct));
    double result = calculate_exponential(x);
    e->expInt = (int)result;
    e->expFraction = (int)((result - e->expInt) * 100);

    // Post-condition: Ensure the structure contains valid values
    if (e->expInt != (int)result) {
        printf("Post-condition failed: expInt is incorrect\n");
        free(e);
        return NULL;
    }
    if (e->expFraction != (int)((result - e->expInt) * 100)) {
        printf("Post-condition failed: expFraction is incorrect\n");
        free(e);
        return NULL;
    }
    return e;
}
