#include <stdio.h>
#include <header.h>


Token token_get(FILE* in){
	int i;
	char temp[IDENT_STR_LEN];
	char* strmem;
	char nextchar=getc(in);
	Token restok;

	//スペース読みとばし
	while(isspace(nextchar)){
		nextchar=getc(in);
	}

	if(isalpha(nextchar)){
		temp[0]=nextchar;
		for(i=1;i<IDENT_STR_LEN;i++){
			nextchar=getc(in);
			if(isalphanum(nextchar)){
				temp[i]=nextchar;
			}else{
				ungetc(in,nextchar);
				break;
			}
		}
		temp[i]=NULL;
		if(isupper(temp[0])){
			restok.tag=VARIABLE;
			restok.value.variable=sym_get(temp);
		}else{
			restok.tag=ATOM;
			restok.value.atom=sym_get(temp);
		}
	}else if(isdigit(nextchar)){
		temp[0]=nextchar;
		for(i=1;i<IDENT_STR_LEN;i++){
			nextchar=getc(in);
			if(isdigit(nextchar)){
				temp[i]=nextchar;
			}else{
				ungetc(in,nextchar);
				break;
			}
		}
		temp[i]=NULL;
		restok.tag=INTEGER;
		restok.value.integer=atoi(temp);
	}else if(nextchar=='?'){
		if((nextchar=getc(in))=='-'){
			restok.tag=QUESTION;
		}else{
			error("lexer: Can't tokenize.");
		}
	}else if(nextchar==':'){
		if((nextchar=getc(in))=='-'){
			restok.tag=IMPLICATION;
		}else{
			error("lexer: Can't tokenize.");
		}
	}else if(nextchar==EOF){
		restok.tag=ENDOFFILE;
	}else{
		restok.tag=ASCII;
		restok.value.ascii=nextchar;
	}

	return restok;
}


