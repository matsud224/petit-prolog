//Mark Sweep GC
#include <malloc.h>
#include <stdio.h>
#include "header.h"
#define CHANK_SIZE (sizeof(char)*512)

GCHeader freelist;
ChankList chanklist;


void gc_init(){
	freelist.fieldsize=0;
	freelist.fieldtag=F_NULL;
	freelist.next=NULL;
	chanklist.next=NULL;

}

void gc_mark_sub(void* m){
	GCHeader* mem;
	if(m!=NULL){
		mem=((GCHeader*)m)-1;
		mem->marked=1;
	}
	switch(mem->fieldtag){
	case F_HISTORYTABLE:
		gc_mark_sub(((HistoryTable*)m)->next);
		gc_mark_sub(((HistoryTable*)m)->ppterm);
		gc_mark_sub(((HistoryTable*)m)->pterm);
		gc_mark_sub(((HistoryTable*)m)->prev);
		break;
	case F_VARIABLETABLE:
		gc_mark_sub(((VariableTable*)m)->next);
		gc_mark_sub(((VariableTable*)m)->variable);
		gc_mark_sub(((VariableTable*)m)->termptr);
		break;
	case F_TERM:
		switch(((Term*)m)->tag){
		case TERM_VARIABLE:
			gc_mark_sub(((Term*)m)->value.variable);
			break;
		case TERM_STRUCTURE:
			gc_mark_sub(((Term*)m)->value.structure);
			break;
		case TERM_PPTERM:
			gc_mark_sub(((Term*)m)->value.ppterm);
			break;
		default:
			break;
		}
		break;
	case F_HTSTACK:
		gc_mark_sub(((HTStack*)m)->next);
		gc_mark_sub(&(((HTStack*)m)->htable));
		break;
	case F_CLAUSE:
		gc_mark_sub(((Clause*)m)->head);
		gc_mark_sub(((Clause*)m)->body);
		break;
	case F_CLAUSELIST:
		gc_mark_sub(((ClauseList*)m)->next);
		gc_mark_sub(((ClauseList*)m)->clause);
		break;
	case F_BOX:
		gc_mark_sub(((Box*)m)->success);
		gc_mark_sub(((Box*)m)->failure);
		gc_mark_sub(((Box*)m)->selected_clause);
		gc_mark_sub(((Box*)m)->vartable);
		gc_mark_sub(((Box*)m)->structure);
		break;
	case F_STRUCTURE:
		gc_mark_sub(((Structure*)m)->functor);
		gc_mark_sub(((Structure*)m)->arguments);
		break;
	case F_TERMLIST:
		gc_mark_sub(((TermList*)m)->next);
		gc_mark_sub(((TermList*)m)->term);
		break;
	case F_PROGRAM:
		gc_mark_sub(((Program*)m)->next);
		switch(((Program*)m)->tag){
		case PROG_CLAUSE:
			gc_mark_sub(((Program*)m)->item.clause);
			break;
		case PROG_QUESTION:
			gc_mark_sub(((Program*)m)->item.question);
			break;
		}
		break;
	case F_STRUCTURELIST:
		gc_mark_sub(((StructureList*)m)->next);
		gc_mark_sub(((StructureList*)m)->structure);
		break;
	case F_SYMBOLTABLE:
		gc_mark_sub(((SymbolTable*)m)->next);
		gc_mark_sub(((SymbolTable*)m)->name);
		gc_mark_sub(((SymbolTable*)m)->clause_list);
		break;
	case F_QUESTION:
		gc_mark_sub(((Question*)m)->body);
		break;
	}
}

void gc_mark(){
	//シンボルテーブルを巡る
	SymbolTable* ptr=symtable;
	while(ptr->next!=NULL){
		gc_mark_sub(ptr->next);
		ptr=ptr->next;
	}

    return;
}

void gc_sweep(){
	ChankList* clist=&chanklist;
    GCHeader* sweeping;
    freelist.next=NULL;
	int first;
    while(clist->next!=NULL){
		sweeping=(GCHeader*)(clist->next->chank);
		first=1;
		while(sweeping<(GCHeader*)(clist->next->chank+CHANK_SIZE)){
			printf("sweep size = %d\n",sweeping->fieldsize);
			if(sweeping->marked==1){
				sweeping->marked=0;
			}else{
				if(!first && freelist.next!=NULL && sweeping==(GCHeader*)(((char*)(freelist.next+1))+sizeof(char)*freelist.next->fieldsize)){
					freelist.next->fieldsize+=sweeping->fieldsize+sizeof(GCHeader);
				}else{
					sweeping->next=freelist.next;
					freelist.next=sweeping;
				}
			}
			first=0;
			sweeping=(GCHeader*)((char*)(sweeping+1)+(sweeping->fieldsize)*sizeof(char));
		}

		clist=clist->next;
    }
}

void gc_freelist_show(){
	GCHeader* ptr=&freelist;

	printf("---freelist---\n");

	while(ptr->next!=NULL){
		printf("%d byte\n",ptr->next->fieldsize);
		ptr=ptr->next;
	}

	printf("--------------\n");
	return;
}

void* gc_malloc(size_t size,FieldTag tag){
	GCHeader* hptr=&freelist;
	int try_count=0;

alloc_retry:
	if(CHANK_SIZE-sizeof(GCHeader)<size){
		error("allocation failed: larger than CHANK_SIZE.");
	}

	while(hptr->next!=NULL){
		if(hptr->next->fieldsize>=size){
			GCHeader* result=hptr->next+1;
			if(hptr->next->fieldsize>size+sizeof(GCHeader)){
				//分割
				size_t beforesize=hptr->next->fieldsize;
				GCHeader* beforenext=hptr->next->next;

				hptr->next->fieldsize=size;
				hptr->next->marked=0;
				hptr->next->fieldtag=tag;
				hptr->next=(GCHeader*)((char*)(hptr->next+1)+(sizeof(char) * size));
				hptr->next->fieldsize=beforesize-size-sizeof(GCHeader);
				hptr->next->next=beforenext;
			}else{
				hptr->next->marked=0;
				hptr->next->fieldtag=tag;
				hptr->next=hptr->next->next;
			}

			return result;
		}
		hptr=hptr->next;
	}

	if(try_count==0){
		try_count=1;
		gc_mark();
		gc_sweep();
		goto alloc_retry;
	}

	gc_chankallocate();
	goto alloc_retry;
}

void gc_chankallocate(){
	ChankList* ptr=&chanklist;
	while(ptr->next!=NULL){
		ptr=ptr->next;
	}
	ptr->next=malloc(sizeof(ChankList));
	if(ptr->next==NULL){
		error("memory allocation failed.");
	}
	ptr->next->next=NULL;
	ptr->next->chank=malloc(CHANK_SIZE);
	if(ptr->next->chank==NULL){
		error("memory allocation failed.");
	}

	GCHeader* hptr=&freelist;
	while(hptr->next!=NULL){
		hptr=hptr->next;
	}

	hptr->next=(GCHeader*)(ptr->next->chank);
	hptr->next->fieldsize=CHANK_SIZE-sizeof(GCHeader);
	hptr->next->marked=0;

	hptr->next->next=NULL;

	printf("GC:allocated new chank.\n");

	return;
}
