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

/** @brief Calculate the approximation of e^x using n terms of the Taylor series expansion.
 * 
 *  @param x The exponent for which the exponential function e^x is approximated.
 * 
 *  @return A pointer to an `ExpStruct` containing the integer and fractional parts of e^x.
 * 
 *  Pre-condition: (e == NULL) and (N == 0)
 * 
 *  Post-condition: If the fraction is out of the range [0, 99], the function returns NULL and prints an error message.
 */
ExpStruct *iexp (int x);

#endif