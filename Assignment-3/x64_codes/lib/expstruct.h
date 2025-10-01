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

struct expStruct {
	int expInt;
	int expFraction;
};

typedef struct expStruct ExpStruct;

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
ExpStruct *iexp(int x);

/* Additional helpers implemented in expstruct.c. They are exposed here
 * because the implementation provides external linkage. These may be
 * useful for testing or reuse.
 */
/** @brief Calculates the factorial of a non-negative integer using recursion and memoization.
 *
 * @details The function first checks for a cached result in `factorial_cache`.
 *          If not found, it computes the factorial recursively (result = x * factorial(x-1))
 *          and stores the result in the cache before returning.
 *          The base case is factorial(0) = 1. It returns 0 for negative input.
 *
 * @param x The non-negative integer for which the factorial is calculated.
 *
 * @return double The factorial of x.
 */
double calculate_factorial(int x);

/** @brief Computes the power of a base raised to an exponent using iteration.
 *
 * @details This function iteratively multiplies the base by itself `exponent` times.
 *          It initializes a result to 1 and multiplies it by the base in a loop.
 *
 * @param base The base value.
 * @param exponent The non-negative exponent.
 *
 * @return double The result of base^exponent.
 */
double power(int base, int exponent);

/* @brief Approximates e^x using a Taylor series expansion.
 *
 * @details It calculates the sum of the series (x^n / n!) term by term.
 *          The loop continues for a maximum of 100 terms or until a term's
 *          value is less than 0.01, ensuring two decimal places of accuracy.
 *          It checks for a cached result in `exp_cache` before computation.
 *
 * @param x The exponent.
 *
 * @return double The approximate value of e^x.
 */
double calculate_exponential(int x);

#endif
