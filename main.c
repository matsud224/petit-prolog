#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <malloc.h>
#include "header.h"


#define MAX_HISTORY 64 //ヒストリの数の上限


//readline　カスタム補完用
char** on_complete(const char* text, int start, int end);
char* word_generator(const char* text, int state);


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

	rl_attempted_completion_function = on_complete; //補完のコールバック関数

	char* p;
	int history_count=0;

    while(1){
        p=readline("> ");
        if(p==NULL){printf("\n");continue;}
        if(*p=='\0'){free(p);continue;}
        FILE* memfile=fmemopen((void*)p,strlen(p),"r");
        interpret(memfile);
        fclose(memfile);
        add_history(p);
        history_count++;
        if(history_count>MAX_HISTORY){
			free(remove_history(0));
			history_count--;
        }
		free(p);
	}
	return 0;
}

char** on_complete(const char* text, int start, int end){
	rl_completion_append_character='\0';
	rl_completer_word_break_characters="\t\n\"\\'`@$><=;|&{(.-,";
	return rl_completion_matches(text, word_generator);
}

char* word_generator(const char* text, int state){
	static int root_index=-1; //シンボルテーブル（親）のインデックス
	static SymbolTable* st_ptr=NULL; //今見てるエントリへのポインタ
	static int wordlen;

	if(state == 0){  // state = 0は初出の単語なので初期化
		root_index=0;
		st_ptr=&(symtable[root_index]);
		wordlen=strlen(text);
	}

	while(root_index<SYMTABLE_LEN){
		while(st_ptr->next!=NULL){
			if(strncmp(st_ptr->next->name,text,wordlen)==0){
				char* result=strdup(st_ptr->next->name);
				st_ptr=st_ptr->next;
				return result;
			}
			st_ptr=st_ptr->next;
		}
		root_index++;
		st_ptr=&(symtable[root_index]);
	}
	return NULL;
}
