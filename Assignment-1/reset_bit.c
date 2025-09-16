#include <stdio.h>
#include <stdlib.h>

typedef struct{
    int content;
} iRegister;

void resetBit(int i, iRegister *r){
    if (r==NULL){
        fprintf(stderr, "Error: A NULL pointer was given to resetBit\n");
        return;
    }
    if (i<0 || i>31) {
        fprintf(stderr, "Error: Invalid bit\n");
        return;
    }

    r->content &= ~(1<<i);

    if ((r->content & (1<<i)) != 0) {
        fprintf(stderr, "Error: Bit %d is not reset\n", i);
        return;
    }

    printf("Bit %d reset successfully\n", i);
}

int main() {
    iRegister r;
    int i;

    printf("Please, enter an integer number: ");
    scanf("%d", &i);
    r.content = 10;
    printf("Entered %d\n", r.content);
    resetBit(1, &r);
    printf("Result of reseting bit 1: %d\n", r.content);
    return 0;
}
