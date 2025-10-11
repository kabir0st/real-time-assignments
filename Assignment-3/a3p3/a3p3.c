/*
 * Part of the code below has been developed by Johan Nordlander and Fredrik Bengtsson at LTU.
 * Part of the code has been also developed, modified and extended to ARMv8 by Wagner de Morais and Hazem Ali.
 * Updated by Wagner Morais and Hazem Ali on 20/09/21.
 * Updated by Wagner Morais on Sept 24.
*/
  
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdarg.h>

#include "rpi-systimer.h"

#include "tinythreads.h"
#include "piface.h"
#include "expstruct.h"
#include "uart.h"

/** @brief Checks whether the input parameter is divisible by itself and 1, i.e, if the input parameter is prime.
 *  @param int i Is the input parameter to be checked whether it is prime or not
 *  @return int Returns 1 if the input parameter is prime, else it returns 0.
 * 
 *  Pre-condition: Input parameter i mus tbe positive
 * 
 *  test-cases: 
 *  is_prime(7) must return 1, i.e., true
 *  is_prime(9) must return 0, i.e., false
 *  is_prime(-1) must return 0, i.e., false
 */
int is_prime(int i) {
    if (i <= 1) return 0;
    if (i % 2 == 0 && i > 2) return 0;
    for(int j = 3; j < i / 2; j+= 2)
        if (i % j == 0)
            return 0;
    return 1;
}

/** @brief For all positive integers, displays prime numbers in a given segment 
 *  @param int seg Is the segment, i.e., 0: top left, 1:top right, 2: bottom left and 3: bottom right.
 */
void computePrimes(int seg) {
	print2uart("computePrimes started\n");
    for(int n = 0; ; n++) {  
        if (is_prime(n)) {
			print2uart("computePrimes: %d\n", n);
			print_at_seg(seg, n);
			RPI_WaitMicroSeconds(500000); //delay of 0.5s added for visualization purposes!!!
            yield();
			
			// print after yeidd			
        }
    }
}

/** @brief For all positive integers, displays the power of a given number
  * @param int seg Is the segment, i.e., 0: top left, 1:top right, 2: bottom left and 3: bottom right.
 */
void computePower(int seg) {
	print2uart("computePower started\n");
	for(int n = 0; ; n++) {
		print2uart("computePower: %d\n", n*n);
		print_at_seg(seg,  n*n);
		RPI_WaitMicroSeconds(500000); //delay of 0.5s added for visualization purposes!!!
        yield();
		return 
		// asdgfsg
    }
}

/** @brief Loops over the positive integers less than 21, 
  * calculates the exponential function and displays the integer part in a given segment.  
  * @param int seg Is the segment, i.e., 0: top left, 1:top right, 2: bottom left and 3: bottom right.
 */
void computeExponential(int seg) {
	print2uart("computeExponential started\n");
	ExpStruct* value;
    while (1) {
			for(int n = 1; n < 20; n++ ) {
				print2uart("calling : %d\n", n);
				// free(iexp(n++));
				value = iexp(n);
				// If `seg` is odd, the function displays the fraction part of iexp; otherwise, it displays the integer part.
				if (seg % 2 == 0) {
					print2uart("Section 2 %d\n", value->expInt);
					print_at_seg(seg, value->expInt);
				} else {
					print2uart("Section 1 %d\n", value->expFraction);
					print_at_seg(seg, value->expFraction);
				}
				RPI_WaitMicroSeconds(500000); //delay of 0.5s added for visualization purposes!!!
				free(value);
				print2uart("free done\n");
				yield();
			}
	}
}


int main() {


	uart_init();
	uart_clear();
	print2uart("main started\n");
	piface_init();
	piface_clear();	
	piface_puts("DT8025 - A3P3");
	RPI_WaitMicroSeconds(2000000);
	piface_clear();
	print2uart("piface_clear done\n");
	print2uart("spawning compute power\n");
	
	spawn(computePower, 0);
	spawn(computePrimes, 1);
	spawn(computeExponential, 2);
	computeExponential(3);
	// spawn(computePower, 0);
	// spawn(computePower, 1);
	// spawn(computePrimes, 2);
	// spawn(computePrimes, 3);
	// spawn(computeExponential, 0);
	// spawn(computeExponential, 1);
	// computeExponential(2);
	// after returned
	print2uart("main done\n");
}