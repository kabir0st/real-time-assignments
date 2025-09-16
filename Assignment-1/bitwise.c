/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Wagner de Morais (Wagner.deMorais@hh.se)
*/
#include <stdlib.h>
#include <stdio.h>

typedef struct{
    int content;
} iRegister;

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
		fprintf(stderr, "Error: Failed to reset Bit!!\n");
		return;
	}
}

int main ()
{
	iRegister r;
	int i;
	printf ("Please, enter an integer number: ");
	// assumed the user will enter a valid integer
	scanf("%d", &i);
	r.content = 10;
	printf ("Entered %d\n", r.content);
	resetBit(1, &r);
	printf ("Result of reseting bit 1: %d\n", r.content);
	return 0;
}
