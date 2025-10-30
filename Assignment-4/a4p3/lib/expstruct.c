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
#include "expstruct.h"
#include <stdio.h>
#include "rpi-systimer.h"
#include "piface.h"

ExpStruct *iexp(int x){
	ExpStruct *e = malloc(sizeof(ExpStruct));
    int n = 1;

    //pre-condition
    if(e == NULL){
        fprintf(stderr, "Error: Malloc failed for variable e in iexp\n");
        return NULL;
    }

    //pre-condition
    if(n == 0){
            e->expFraction = 0;
            e->expInt = 1;
            return e;
    }

    double den = 1.0;
    double num = 1.0;
    double sum = 1.0;
    double prevV = 1.0;
    double currV = 1.0;
    
   do{
        prevV = currV;
        den *= n;
        num *= x;
        sum += num/den;
        currV = sum;
        n++;
    	//RPI_WaitMicroSeconds(25000);
   }while((currV - prevV) > 0.01 && n < 55);

    e->expInt = (int) sum;
    e->expFraction = (int)((sum - e->expInt) * 100);

    //post-condition
    if( 100 <= e->expFraction || e->expFraction < 0){
        fprintf(stderr, "The fraction is out of range [0, 99]\n");
        return NULL;
    }
	return e;	
}
