#include <stdio.h>
#include "header.h"


int main(int argc,char* argv[]){
	Token gottoken;
    while(1){
        printf(">");
        while((gottoken=token_get(stdin)).tag!=TOK_ENDOFFILE){
            switch(gottoken.tag){
            case TOK_ASCII:
                printf("ASCII:%c\n",gottoken.value.ascii);
            break;
            case TOK_ATOM:
                printf("ATOM:%p\n",gottoken.value.atom);
            break;
            case TOK_ENDOFFILE:
                printf("EOF\n");
            break;
            case TOK_IMPLICATION:
                printf(":-\n");
            break;
            case TOK_INTEGER:
                printf("INTEGER:%d\n",gottoken.value.integer);
            break;
            case TOK_QUESTION:
                printf("?-\n");
            break;
            case TOK_VARIABLE:
                printf("VARIABLE:%p\n",gottoken.value.variable);
            break;
            default:
                printf("<UNKNOWN TOKEN>\n");
            break;
            }
        }
        printf("\n");
	}
	return 0;
}
