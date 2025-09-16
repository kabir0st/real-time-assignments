#include <stdlib.h>
#include <stdio.h>
#include "lib/iregister.h"

int main(){

    iRegister r;
    r.content = 10;
    printf("r.content: %d\n", r.content);
    resetBit(1, &r);
    printf("After reseting: %d\n", r.content);
    return 0;
}
