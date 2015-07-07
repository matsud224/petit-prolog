#include <malloc.h>
#include <header.h>


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
	//検索し、ポインタを返す。なければ登録も行う
	SymbolTable* searchptr=&symtable[sym_hash(str)];
	while(searchptr->next!=NULL){
		if(strcmp(searchptr->next->name,str)==0){
			return searchptr->next;
		}else{
			searchptr=searchptr->next;
		}
	}
	//見つからなかった...
	searchptr->next=malloc(sizeof(SymbolTable));
	searchptr->next->name=malloc(strlen(str)+1);
	strcpy(searchptr->next->name,str);
	return searchptr->next;
}

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
	//検索し、ポインタを返す。なければ登録も行う
	SymbolTable* searchptr=&symtable[sym_hash(str)];
	while(searchptr->next!=NULL){
		if(strcmp(searchptr->next->name,str)==0){
			return searchptr->next;
		}else{
			searchptr=searchptr->next;
		}
	}
	//見つからなかった...
	searchptr->next=malloc(sizeof(SymbolTable));
	searchptr->next->name=str;
	return searchptr->next;
}
