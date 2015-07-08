#include <stdio.h>
#include "header.h"


int main(int argc,char* argv[]){
    while(1){
        printf(">");
        interpret(stdin);
	}
	return 0;
}
