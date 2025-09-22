
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
	if( i <= 0 || i >= 31)
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
    printf("r.content: %d\n", r.content);
    static char str[33]; // allocate 33 characters, 32 bits + null terminator
    printf("str: %s\n", &str);
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


// for testing and understanding the binary representation of the value
void convert_to_binary(int value) {
    int bits = sizeof(value) * 8; // usually 32 bits
    char *bin_str = malloc(bits + 1);  // +1 for null terminator
    if (!bin_str) return;         // check allocation
    for (int i = bits - 1; i >= 0; i--) {
        // check if the bit is set
        bin_str[bits - 1 - i] = (value & (1 << i)) ? '1' : '0';
    }
    bin_str[bits] = '\0'; // null terminate the string
    fprintf(stderr, "Binary string: %s\n", bin_str);
    free(bin_str); // free allocated memory
}

void shiftRight(int n, iRegister *r){
    // pre-condition
    if (r == NULL || n <= 0 || n >= 31) {
        fprintf(stderr, "Error: Invalid shift amount or NULL pointer\n");
        return;
    };
    // we did a arthematic shift right
    // so we can preseve the sign value of the
    // number, if we needed to preserve the structure
    // (addeding 0 to the left instaed of 1 ) we would
    // tyepcast the r->content to unsigned int
    // and then shift right
    int old_value = r->content;

    r->content >>= n;
    // post-condition: simple verification
    if (r->content != (old_value >> n)) {
        fprintf(stderr, "Error: Failed to shift right\n");
        return;
    }
}

void shiftLeft(int n, iRegister *r){
    // pre-condition
    if (r == NULL || n <= 0 || n >= 31) {
        fprintf(stderr, "Error: Invalid shift amount or NULL pointer\n");
        return;
    };
    // left shift, fills with 0
    int old_value = r->content;

    r->content <<= n;
    // post-condition

    // post-condition
    if (r->content != (old_value << n)) {
        fprintf(stderr, "Error: Failed to shift left\n");
        return;
    }
}

void setBit(int i, iRegister *r) {
    // pre-condition
    if (r == NULL || i <= 0 || i >= 31) {
        fprintf(stderr, "Error: A NULL pointer or invalid bit was given to setBit\n");
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
    if (r == NULL || i <= 0 || i >= 31) {
        fprintf(stderr, "Error: A NULL pointer or invalid bit was given to getBit\n");
        return -1;
    }
    // pre-condition
    // get the bit
    return (r->content & (1 << i)) ? 1 : 0;
}

int getNibble(int pos, iRegister *r) {
    // pre-condition
    if (r == NULL || (pos <= 0 || pos >= 7)) {
        fprintf(stderr, "Error in getNibble\n");
        return -1;
    }
    // get the nibble
    return (r->content >> 4*pos) & 0xF; //shift right 4 and mask 1111 to get bits 4-7
    // no post-condition because register is not modified
}

void assignNibble(int value, int pos, iRegister *r) {
    // pre-condition
    if (r == NULL || (pos <= 0 || pos >= 7)) {
        fprintf(stderr, "Error: Invalid parameters given to assignNibble\n");
        return;
    }
    // pre-condition: check if value is valid nibble (0-15)
    if (value <= 0 || value >= 15) {
        fprintf(stderr, "Error: Invalid nibble value (must be 0-15)\n");
        return;
    }

    // Clear the target nibble first, then set the new value
    int shift = 4 * pos;                    // Calculate bit position
    int mask = 0xF << shift;                // Create mask for the nibble
    r->content &= ~mask;                    // Clear the target nibble
    r->content |= (value << shift);         // Set the new nibble value

    // post-condition: verify the nibble was set correctly using getNibble
    int retrieved_value = getNibble(pos, r);
    if (retrieved_value != value) {
        fprintf(stderr, "Error: Failed to assign nibble. Expected %d, got %d\n", value, retrieved_value);
        return;
    }
}
