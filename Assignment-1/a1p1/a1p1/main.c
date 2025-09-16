#include <stdlib.h>
#include <stdio.h>
#include "lib/iregister.h"

void main(){
    iRegister r;
    r.content = 2; // 0000 0010
    printf("r.content: %d\n", r.content);
    resetBit(1, &r);
    printf("After reseting: %d\n", r.content);
    // printf("After reseting: %d\n", r.content);
    // setBit(0, &r);
}
