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

char* reg2str(iRegister r) {

    // pre-condition - removed NULL check since r is passed by value

    // range of two's complement system is -2^(n-1) to 2^(n-1)-1
    // We need to allocate bits + 1 for the null terminator

    // null termination is used to add a null character at the end of the string
    // this is used to indicate the end of the string
    // avoid undefined behavior among other things

    static char str[33]; // allocate 33 characters, 32 bits + null terminator

    for (int i = 31; i >= 0; i--) {
        // doing right shift from the MSB to the LSB
        // and then checking if the bit is set
        // if it is set, then the character is "1"
        // if it is not set, then the character is "0"
        str [31 - i] = (r.content >> i) & 1u ? '1' : '0';
    }
    str[32] = '\0'; // null terminator for string

    // post-condition: verify the string is properly formatted
    if ( str[32] != '\0') {
        fprintf(stderr, "Error: Failed to create proper string representation\n");
        return NULL;
    }
    // post-condition: verify string length is exactly 32 characters
    int len = 0;
    for (int i = 0; i < 32; i++) {
        if (str[i] != '0' && str[i] != '1') {
            fprintf(stderr, "Error: Invalid character in binary string\n");
            return NULL;
        }
        len++;
    }
    if (len != 32) {
        fprintf(stderr, "Error: String length is not 32 characters\n");
        return NULL;
    }

    return str;
}

void convert_to_binary(int value) {
    int bits = sizeof(value) * 8; // usually 32 bits
    char *bin_str = malloc(bits + 1);  // +1 for null terminator
    if (!bin_str) return;         // check allocation
    for (int i = bits - 1; i >= 0; i--) {
        bin_str[bits - 1 - i] = (value & (1 << i)) ? '1' : '0';
    }
    bin_str[bits] = '\0'; // null terminate the string
    fprintf(stderr, "Binary string: %s\n", bin_str);
}

void shiftRight(int n, iRegister *r){
    // pre-condition
    if (r == NULL || n < 0 || n > 31) return;
    int temp = (int) r->content;
    fprintf(stderr, "Shift right B : ");
    convert_to_binary(temp);
    // logical shift, fills with 0
    temp >>= n;
    fprintf(stderr, "Shift right A : ");
    convert_to_binary(temp);
    // store back
    r->content = (int) temp;
    // post-condition
}

void shiftLeft(int n, iRegister *r){
    // pre-condition
    if (r == NULL || n < 0 || n > 31) return;
    // store original value for post-condition check
    fprintf(stderr, "Shift left B : ");
    convert_to_binary(r->content);
    int temp = r->content;
    // left shift, fills with 0
    r->content <<= n;
    fprintf(stderr, "Shift left A : ");
    convert_to_binary(r->content);
    // post-condition
    if (r->content != (temp << n)) {
        fprintf(stderr, "Error: Failed to shift left\n");
        return;
    }
}

void setBit(int i, iRegister *r) {
    // pre-condition
    if (r == NULL || i < 0 || i > 31) {
        fprintf(stderr, "Error: A NULL pointer was given to setBit\n");
        return;
    }
    // pre-condition
    // set the bit to 1
    // bitwise OR shifts 1 to the left i times and then
    //  ORs it with the content of register
    r->content |= (1 << i);
    // post-condition
    // checking if the bit is set to 1
    if ((r->content & (1 << i)) == 0) {
        fprintf(stderr, "Error: Failed to set Bit\n");
        return;
    }
}


void setAll(iRegister *r) {
    // pre-condition
    if (r == NULL) {
        fprintf(stderr, "Error: A NULL pointer was given to setAll\n");
        return;
    }
    // set all the bits to 1
    r->content = -1;
    // post-condition
    if (r->content != -1) {
        fprintf(stderr, "Error: Failed to set All\n");
        return;
    }
}


int getBit(int i, iRegister *r) {
    // pre-condition
    if (r == NULL || i < 0 || i > 31) {
        fprintf(stderr, "Error: A NULL pointer was given to getBit\n");
        return -1;
    }
    // pre-condition
    // get the bit
    return (r->content & (1 << i)) ? 1 : 0;
}

int getNibble(int pos, iRegister *r) {
    // pre-condition
    if (r == NULL || (pos != 1 && pos != 2)) {
        fprintf(stderr, "Error: A NULL pointer was given to getNibble\n");
        return -1;
    }
    // get the nibble
    if(pos == 1){
        return r->content & 0xF; //mask 1111 to get lowest 4 bits
    }
     //pos == 2
    return (r->content >> 4) & 0xF; //shift right 4 and mask 1111 to get bits 4-7
}

void assignNibble(int value, int pos, iRegister *r) {
    // pre-condition
    if (r == NULL || (pos != 1 && pos != 2)) {
        fprintf(stderr, "Error: A NULL pointer was given to assignNibble\n");
        return;
    }
    // pre-condition
    // assign the nibble
    r->content |= (value << (pos == 1 ? 0 : 4));
    // post-condition
    if (r->content != (value << (pos == 1 ? 0 : 4))) {
        fprintf(stderr, "Error: Failed to assign nibble\n");
        return;
    }
}
