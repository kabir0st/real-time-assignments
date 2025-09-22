#include <stdlib.h>
#include <stdio.h>
#include "lib/iregister.h"

int main(){

    iRegister r;
    iRegister e;
    r.content = 85;
    e.content = 2;

    printf("%s %s  \n", reg2str(r),  reg2str(e));
    // this works but another does not
    // because it only gets the address and when
    // printing the printf access the address
    // not the value, it doesn't copy the current value
    // to it's buffer, it uses what is in the address
    // and not the current value
    printf("%s  \n", reg2str(r));
    printf("%s  \n", reg2str(e));
}
