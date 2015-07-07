#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <header.h>


int main(int argc,char* argv[]){
	printf("%p\n",sym_get("hello"));
	printf("%p\n",sym_get("world"));
	printf("%p\n",sym_get("hello"));
	return 0;
}
