#include <stdlib.h>
#include <stdio.h>
#include "lib/iregister.h"

int main(){

    iRegister r;
    iRegister e;
    r.content = 85;
    e.content = 2;
    char *s = reg2str(r);
    char *t = reg2str(e);
    printf("%s %s  \n", s, t);
    free(s);
    free(t);
    // this works but another does not
    // because it only gets the address and when
    // printing the printf access the address
    // not the value, it doesn't copy the current value
    // to it's buffer, it uses what is in the address
    // and not the current value
}
