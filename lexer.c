#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "header.h"


Token token_get(FILE* in){
	int i;
	char temp[IDENT_MAX_STR_LEN];
	char nextchar=getc(in);
	Token restok;

	//スペース読みとばし
	while(isspace(nextchar)){
		nextchar=getc(in);
	}

	if(isalpha(nextchar)){
		temp[0]=nextchar;
		for(i=1;i<IDENT_MAX_STR_LEN;i++){
			nextchar=getc(in);
			if(isalnum(nextchar)){
				temp[i]=nextchar;
			}else{
				ungetc(nextchar,in);
				break;
			}
		}
		temp[i]='\0';
		if(isupper(temp[0])){
			restok.tag=TOK_VARIABLE;
			restok.value.variable=sym_get(temp);
		}else{
			restok.tag=TOK_ATOM;
			restok.value.atom=sym_get(temp);
		}
	}else if(isdigit(nextchar)){
		temp[0]=nextchar;
		for(i=1;i<IDENT_MAX_STR_LEN;i++){
			nextchar=getc(in);
			if(isdigit(nextchar)){
				temp[i]=nextchar;
			}else{
				ungetc(nextchar,in);
				break;
			}
		}
		temp[i]='\0';
		restok.tag=TOK_INTEGER;
		restok.value.integer=atoi(temp);
	}else if(nextchar=='?'){
		if((nextchar=getc(in))=='-'){
			restok.tag=TOK_QUESTION;
		}else{
			error("lexer: Can't tokenize.");
		}
	}else if(nextchar==':'){
		if((nextchar=getc(in))=='-'){
			restok.tag=TOK_IMPLICATION;
		}else{
			error("lexer: Can't tokenize.");
		}
	}else if(nextchar==EOF){
		restok.tag=TOK_ENDOFFILE;
	}else{
		restok.tag=TOK_ASCII;
		restok.value.ascii=nextchar;
	}

	return restok;
}


