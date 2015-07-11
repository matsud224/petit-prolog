#include <stdio.h>
#include "header.h"


int main(int argc,char* argv[]){
    while(1){
        printf(">");
        interpret(stdin);
        printf("\n");
	}
	return 0;
}
