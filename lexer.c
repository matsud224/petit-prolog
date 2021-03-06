#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "header.h"

#define SEEN_TOKEN_STACK_SIZE 20
Token seen_token_stack[SEEN_TOKEN_STACK_SIZE]; //先読みした後に押し戻されたトークンをためておくスタック(溢れないように最大押し戻し回数分確保)
int seen_token_count=0;



Token token_get(FILE* fp){
	int i;
	char temp[IDENT_MAX_STR_LEN];
	int nextchar;
	Token restok;

	if(seen_token_count>0){
        return seen_token_stack[--seen_token_count];
	}

restart:

	nextchar=getc(fp);

	//スペース読みとばし
	while(isspace(nextchar)){
		nextchar=getc(fp);
	}

	if(isalpha(nextchar) || nextchar=='_' || nextchar=='!' || nextchar=='-' || nextchar=='+' || nextchar=='*' || nextchar=='/'){
		temp[0]=nextchar;
		for(i=1;i<IDENT_MAX_STR_LEN;i++){
			nextchar=getc(fp);
			if(isalnum(nextchar) || nextchar=='_'){
				temp[i]=nextchar;
			}else{
				ungetc(nextchar,fp);
				break;
			}
		}
		temp[i]='\0';

		if(temp[0]=='!' || temp[0]=='-' || temp[0]=='+' || temp[0]=='*' || temp[0]=='/'){
			if(temp[1]!='\0'){
				if(temp[0]=='-' || temp[0]=='+'){
					goto gentoken_int;
				}else{
					error("Can't tokenize.");
				}
			}
			restok.tag=TOK_ATOM;
			restok.value.atom=sym_get(temp);
		}else if(isupper(temp[0]) || temp[0]=='_'){
			if(temp[0]=='_' && temp[1]=='\0'){
				//アンダースコア単独の場合...名前無し変数
				restok.tag=TOK_VARIABLE;
				restok.value.variable=sym_get_anonymousvar();
			}else{
				restok.tag=TOK_VARIABLE;
				restok.value.variable=sym_get(temp);
			}
		}else{
			restok.tag=TOK_ATOM;
			restok.value.atom=sym_get(temp);
		}
	}else if(isdigit(nextchar)){
		temp[0]=nextchar;
		for(i=1;i<IDENT_MAX_STR_LEN;i++){
			nextchar=getc(fp);
			if(isdigit(nextchar)){
				temp[i]=nextchar;
			}else{
				ungetc(nextchar,fp);
				break;
			}
		}
		temp[i]='\0';
gentoken_int:
		restok.tag=TOK_INTEGER;
		restok.value.integer=atoi(temp);
	}else if(nextchar=='%'){
		while(nextchar!='\n'){
			nextchar=getc(fp);
		}
		goto restart;
	}else if(nextchar=='?'){
		if((nextchar=getc(fp))=='-'){
			restok.tag=TOK_QUESTION;
		}else{
			error("Can't tokenize.");
		}
	}else if(nextchar==':'){
		if((nextchar=getc(fp))=='-'){
			restok.tag=TOK_IMPLICATION;
		}else{
			error("Can't tokenize.");
		}
	}else if(nextchar==EOF){
		restok.tag=TOK_ENDOFFILE;
	}else{
		restok.tag=TOK_ASCII;
		restok.value.ascii=nextchar;
	}

	return restok;
}

void token_unget(Token t){
    if(seen_token_count==SEEN_TOKEN_STACK_SIZE){
        error("internal stack overflow.");
    }
    seen_token_stack[seen_token_count++]=t;
}
