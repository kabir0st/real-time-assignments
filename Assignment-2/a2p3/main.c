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

int main()
{
    ExpStruct* value;
    // for testing
    while(1){
        for (int i = 0; i < 21; i++) {
            value = iexp(i);
            printf("e^%d = %d.%02d\n", i, value->expInt, value->expFraction);
        }
    }
    free(value);
	return 0;
}
