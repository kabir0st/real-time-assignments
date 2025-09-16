#include <stdlib.h>
#include <stdio.h>
#include "lib/iregister.h"

int main(){

    iRegister r;
    r.content = 10;
    printf("r.content: %s = %d\n", convert_to_binary(&r),r.content);

    r.content &= ~(1 << 5);

    printf("r.content: %s = %d\n", convert_to_binary(&r), r.content);
    // resetBit(1, &r);
    // printf("After reseting: %d\n", r.content);
    // printf("--------------------------------\n");

    // r.content = 10;
    // printf("r.content: %d\n", r.content);
    // resetAll(&r);
    // printf("After reseting: %d\n", r.content);
    // printf("--------------------------------\n");

    return 0;
}
