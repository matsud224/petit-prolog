#include "header.h"
#include <stdio.h>

void error(char* msg){
    fprintf(stderr,"ERROR: %s\n",msg);
    return;
}
