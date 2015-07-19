#include <string.h>
#include <malloc.h>
#include "header.h"


SymbolTable symtable[SYMTABLE_LEN]={{NULL,NULL,NULL}};
int anonvar_id=0;

int sym_hash(char* str){
	char* curr=str;
	int value=0;
	while(*curr!='\0'){
		value+=*curr;
		curr++;
	}
	return value%64;
}

SymbolTable* sym_get(char* str){
	gcmemstack_pushnew(&GCMEMSTACK);
	//検索し、ポインタを返す。なければ登録も行う
	SymbolTable* searchptr=&symtable[sym_hash(str)];
	while(searchptr->next!=NULL){
		if(strcmp(searchptr->next->name,str)==0){
			gcmemstack_pop(&GCMEMSTACK);
			return searchptr->next;
		}else{
			searchptr=searchptr->next;
		}
	}
	//見つからなかった...
	searchptr->next=gc_malloc(sizeof(SymbolTable),F_SYMBOLTABLE,&GCMEMSTACK);
	searchptr->next->next=NULL;
	searchptr->next->clause_list=gc_malloc(sizeof(ClauseList),F_CLAUSELIST,&GCMEMSTACK);
	searchptr->next->clause_list->next=NULL;
	searchptr->next->name=gc_malloc(strlen(str)+1,F_CHARARRAY,&GCMEMSTACK);
	strcpy(searchptr->next->name,str);

	gcmemstack_pop(&GCMEMSTACK);
	return searchptr->next;
}

SymbolTable* sym_get_anonymousvar(){
	char str[256];
	sprintf(str,"%%anonymous%d",anonvar_id);
	anonvar_id++;
	return sym_get(str);
}
