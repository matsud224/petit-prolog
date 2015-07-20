#include <stdio.h>
#include <readline/readline.h>
#include <malloc.h>
#include "header.h"


int main(int argc,char* argv[]){
	CURRENT_BEGINBOX=NULL;
	GCMEMSTACK.next=NULL;
/*
	gcmemstack_pushnew(&GCMEMSTACK);
	gcmemstack_pushnew(&GCMEMSTACK);
	gcmemlist_add(gcmemstack_top(&GCMEMSTACK),0x11);
	gcmemlist_add(gcmemstack_top(&GCMEMSTACK),0x11);
	gcmemstack_returnptr(0x12,&GCMEMSTACK);
	printf("%d",gcmemlist_size(gcmemstack_top(&GCMEMSTACK)));
	gcmemstack_pop(&GCMEMSTACK);
	printf("%d",gcmemstack_size(&GCMEMSTACK));

	return 0;
*/

	gc_init();

	char* p;

    while(1){
        p=readline(">");
        FILE* memfile=fmemopen((void*)p,strlen(p),"r");
        interpret(memfile);
        fclose(memfile);
		free(p);
	}
	return 0;
}
