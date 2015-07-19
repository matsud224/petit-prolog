#include <stdio.h>
#include "header.h"


int main(int argc,char* argv[]){
	CURRENT_BEGINBOX=NULL;
	GCMEMSTACK.next=NULL;
/*
	gcmemstack_pushnew(&GCMEMSTACK);
	gcmemstack_pushnew(&GCMEMSTACK);
	gcmemstack_pushnew(&GCMEMSTACK);
	gcmemlist_add(gcmemstack_top(&GCMEMSTACK),0x11);
	gcmemlist_add(gcmemstack_top(&GCMEMSTACK),0x11);
	gcmemstack_returnptr(0x12,&GCMEMSTACK);
	printf("%d",gcmemlist_size(gcmemstack_top(&GCMEMSTACK)));
	gcmemstack_pop(&GCMEMSTACK);
	printf("%d",gcmemlist_size(gcmemstack_top(&GCMEMSTACK)));

	return 0;
*/
	gc_init();

    while(1){
        printf(">");
        interpret(stdin);
        printf("\n");
	}
	return 0;
}
