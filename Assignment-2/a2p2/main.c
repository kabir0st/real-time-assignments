/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Copyright (c) 2017, Sebastian Kunze <sebastian.kunze@hh.se>
    All rights reserved.
	Wagner de Morais (Wagner.deMorais@hh.se)
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lib/expstruct.h"
// #include "piface.h"

#define LINE 32

int main()
{
    printf("Hello\n");
    ExpStruct *exp_result;
    double actual = 1;
    for (int i = 0; i <= 20; i++) {
        exp_result = iexp(i);
        actual = (exp(i));
        printf("x = %d : Exp Aprox: %d.%02d  : Actual: %.2f\n", i, exp_result->expInt, exp_result->expFraction, actual);
    }
    free(exp_result);
    return 0;
}
