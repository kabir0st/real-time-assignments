/*
 * Part of the code below has been developed by Johan Nordlander and Fredrik Bengtsson at LTU.
 * Part of the code has been also developed, modified and extended to ARMv8 by Wagner de Morais and Hazem Ali.
 * Updated by Wagner Morais and Hazem Ali on 20/09/21.
 * Updated by Wagner Morais on Sept 24.
 * Adapted for x86_64/Linux for testing and development
*/

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#include "lib/tinythreads.h"
#include "lib/expstruct.h"


/** @brief Checks whether the input parameter is divisible by itself and 1, i.e, if the input parameter is prime.
 *  @param int i Is the input parameter to be checked whether it is prime or not
 *  @return int Returns 1 if the input parameter is prime, else it returns 0.
 *
 *  Pre-condition: Input parameter i must be positive
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
    for(int n = 0; ; n++) {
        if (is_prime(n)) {
            printf("T%d: Prime %d\n", seg, n);
			usleep(500000); //delay of 0.5s added for visualization purposes!!!
            yield();
        }
    }
}

/** @brief For all positive integers, displays the power of a given number
  * @param int seg Is the segment, i.e., 0: top left, 1:top right, 2: bottom left and 3: bottom right.
 */
void computePower(int seg) {
	for(int n = 0; ; n++) {
		printf("T%d: %d^2=%d\n", seg, n, n*n);
		usleep(500000); //delay of 0.5s added for visualization purposes!!!
        yield();
    }
}

/** @brief Loops over the positive integers less than 21,
  * calculates the exponential function and displays the integer part in a given segment.
  * @param int seg Is the segment, i.e., 0: top left, 1:top right, 2: bottom left and 3: bottom right.
 */
void computeExponential(int seg) {
	ExpStruct* value;
    while (1) {
		for(int n = 1; n < 20; ) {
			value = iexp(n++);

			// If `seg` is odd, the function displays the fraction part of iexp; otherwise, it displays the integer part.
			if (seg % 2 == 0) {
				printf("T%d: e^%d=%d.%02d (int part: %d)\n", seg, n-1, value->expInt, value->expFraction, value->expInt);
			} else {
				printf("T%d: e^%d=%d.%02d (frac part: 0.%02d)\n", seg, n-1, value->expInt, value->expFraction, value->expFraction);
			}

			usleep(500000); //delay of 0.5s added for visualization purposes!!!
			free(value);
			yield();
		}
	}
}


int main() {

	printf("=================================\n");
	printf("DT8025 - A3P1 (x64 Linux Version)\n");
	printf("=================================\n");
	usleep(2000000);
	printf("\nStarting threads...\n\n");

	spawn(computePrimes, 0);
    spawn(computePrimes, 1);
    spawn(computePrimes, 2);
	spawn(computePrimes, 3);
    spawn(computePrimes, 4);

	// computePrimes(1);
	while(1) {
		yield();
	}
	return 0;
}
