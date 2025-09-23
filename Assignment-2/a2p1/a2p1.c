/*
	Modified by Wagner Morais on Sep 2023.
 */

#include <stdio.h>
#include "piface.h"
#include "rpi-systimer.h"


int main()
{
	piface_init();
	piface_clear();
	piface_puts("Waiting\n");
	RPI_WaitMicroSeconds(2000000);
    piface_clear();
    piface_puts("Hawing\n");
    RPI_WaitMicroSeconds(2000000);
    piface_clear();
	return 0;
}
