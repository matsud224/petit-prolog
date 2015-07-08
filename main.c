#include <stdio.h>
#include "header.h"


int main(int argc,char* argv[]){
	Program p;
	Program* current_line;
    while(1){
        printf(">");
        p=parse_program(stdin);
        current_line=&p;
        while(current_line->next!=NULL){
            switch(current_line->next->tag){
            case PROG_CLAUSE:
                printf("CLAUSE: \n");
                break;
            case PROG_QUESTION:
                printf("QUESTION: \n");
                break;
            default:
                printf("UNKNOWN: \n");
                break;
            }

            current_line=current_line->next;
        }
        printf("\n");
	}
	return 0;
}
