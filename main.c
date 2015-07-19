#include <stdio.h>
#include "header.h"


int main(int argc,char* argv[]){
	gc_init();

    while(1){
		gc_mark();
		gc_sweep();
		//gc_freelist_show();
        printf(">");
        interpret(stdin);
        printf("\n");
	}
	return 0;
}
