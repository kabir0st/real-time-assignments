/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Wagner Morais on Aug 2022.
 */

#include "led.h"
#include "rpi3.h"
#include "rpi-systimer.h"
#include "rpi-gpio.h"

void led_init(){
	/* Write 1 to the GPIO16 init nibble in the Function Select 1 GPIO
       peripheral register to enable GPIO16 as an output */

       // changed to GPIO36

    GPIO->GPFSEL3 |=  (1 << 18);   // set to output

#if defined( RPI3 ) && defined( IOBPLUS )
	RPI_SetGpioPinFunction( LED_GPIO, FS_OUTPUT );
#endif
}

void led_on(){
	/* Set the GPIO16 output high ( Turn OK LED on )*/
	GPIO->GPSET1 = (1u << 4);
#if defined( RPI3 ) && defined( IOBPLUS )
	/* Set the ACT LED attached to output high ( Turn OK LED on )
	   Declarations in rpi-gpio.h
	*/
	GPIO->LED_GPSET |= (1u << LED_GPIO_BIT);
#endif
}

void led_off(){
	/* Set the GPIO16 output low ( Turn OK LED off )*/
	GPIO->GPCLR1 = (1u << 4);
#if defined( RPI3 ) && defined( IOBPLUS )
	/* Set the ACT LED attached to output low ( Turn OK LED off )
	   Declarations in rpi-gpio.h
	*/
	GPIO->LED_GPCLR |= (1u << LED_GPIO_BIT);
#endif
}

void led_toggle(){
    if (GPIO->GPLEV1 & (1u << 4)) {
        // LED is currently on, turn it off
        led_off();
    } else {
        // LED is currently off, turn it on
        led_on();
    }
}


void led_blink(){
    while(1){
        led_toggle();
        RPI_WaitMicroSeconds(1000000);
    }
}
