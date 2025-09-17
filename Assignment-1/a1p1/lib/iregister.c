//  Created by Mohammadreza Mousavi [mohmou] on 9/5/14.
//  Updated by Masoumeh Taromirad on 11/08/16.
//  Updated by Wagner Morais and Johannes van Esch on 28/08/18.
//  Updated by Wagner Morais and Hazem Ali on 26/08/21.
//  Copyright (c) 2014 by Mohammadreza Mousavi [mohmou]. All rights reserved.

#include <stdlib.h>
#include <stdio.h>
#include "iregister.h"

void resetBit(int i, iRegister *r)
{
	// pre-condition
	if(r == NULL)
	{
		fprintf(stderr, "Error: A NULL pointer was given to resetBit\n");
		return;
	}
	// pre-condition
	if( i < 0 || i > 31)
	{
		fprintf(stderr,"Error: Invalid bit\n");
		return;
	}

  	r->content &= ~(1 << i);

	// post-condition
	if((r->content & (1<<i)) != 0)
	{
		fprintf(stderr, "Error: Failed to reset Bit\n");
		return;
	}
}

void resetAll(iRegister *r) {
    // check if the pointer is NULL
    if (r == NULL) {
        fprintf(stderr, "Error: A NULL pointer was given to resetAll\n");
        return;
    }
    // reset all the bits to 0
    r->content = 0;
    // check if the bits are reset saved in the register
    if (r->content != 0) {
        fprintf(stderr, "Error: Failed to reset All\n");
        return;
    }
}
// Function to return binary string of r->content
// I copied this function from the internet, I am trying
// to understand why the malloc
char* convert_to_binary(iRegister *r) {
    // this converts byte to bits
    // range of 2 complement system is -2^(n-1) to 2^(n-1)-1
    // so we need to allocate bits + 1 for the sign bit
    // here -2^31 to 2^31-1
    // in a 3 bit system it's going to be -4 to +3
    // check notes on usages of MSB and having to
    // eleminaite +0 and -0.

    int bits = sizeof(r->content) * 8; // usually 32 bits

    // null termination is used to add a null character at the end of the string
    // this is used to indicate the end of the string
    // avoid undefined behavior among other things that I've not
    // understood yet.
    char *bin_str = malloc(bits + 2);  // +1 for sign, +1 for null terminator
    if (!bin_str) return NULL;         // check allocation
    // if nothing is allocated just return Null.

    // Add sign character at the beginning
    bin_str[0] = (r->content < 0) ? '-' : '+';

    // Convert all bits including the sign bit (MSB)
    for (int i = bits - 1; i >= 0; i--) {
        bin_str[bits - i] = (r->content & (1 << i)) ? '1' : '0';
    }
    bin_str[bits + 1] = '\0'; // null terminate the string
    return bin_str;
}
