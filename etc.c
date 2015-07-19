#include <stdio.h>
#include <string.h>
#include "header.h"



void error(char* msg){
    fprintf(stderr,"ERROR: %s\n",msg);
    return;
}


void htable_add(HistoryTable* ht,Term* pterm){
	gcmemstack_pushnew(&GCMEMSTACK);
	HistoryTable* ptr=ht;
	//printf("~~~htable added.~~~\n");
	//後入れ先出し
	HistoryTable* temp=ptr->next;
	ptr->next=gc_malloc(sizeof(HistoryTable),F_HISTORYTABLE,&GCMEMSTACK);
	ptr->next->pterm=pterm;
	ptr->next->prev=NULL;
	ptr->next->next=temp;
	gcmemstack_pop(&GCMEMSTACK);
	return;
}
void htable_addforward(HistoryTable* ht,VariableTable* ppterm,Term* prev){
	gcmemstack_pushnew(&GCMEMSTACK);

	HistoryTable* ptr=ht;
	//printf("~~~htable added(forward).~~~\n");
	//後入れ先出し

	HistoryTable* temp=ptr->next;
	ptr->next=gc_malloc(sizeof(HistoryTable),F_HISTORYTABLE,&GCMEMSTACK);
	ptr->next->ppterm=ppterm;
	ptr->next->prev=prev;
	ptr->next->next=temp;
	gcmemstack_pop(&GCMEMSTACK);
	return;
}

void vartable_addvar(VariableTable *vl,Variable var){
	gcmemstack_pushnew(&GCMEMSTACK);

	VariableTable* ptr=vl;

	while(ptr->next!=NULL){
		if(ptr->next->variable==var){
			//重複を見つけた
			gcmemstack_pop(&GCMEMSTACK);
			return;
		}
		ptr=ptr->next;
	}

	ptr->next=gc_malloc(sizeof(VariableTable),F_VARIABLETABLE,&GCMEMSTACK);
	ptr->next->variable=var;

	ptr->next->termptr=gc_malloc(sizeof(Term),F_TERM,&GCMEMSTACK);
	ptr->next->termptr->tag=TERM_UNBOUND;
	ptr->next->next=NULL;
	gcmemstack_pop(&GCMEMSTACK);
	return;
}

void vartable_show(VariableTable v1){
	VariableTable* ptr=&v1;

	printf("\n-----------------\n");

	while(ptr->next!=NULL){
		if(ptr->next->variable->name[0]!='%'){
			printf("%s = ",ptr->next->variable->name);
			term_show(ptr->next->termptr);
			//printf("(%d)",ptr->next->value.ref_bound);
			printf("\n");
		}

		ptr=ptr->next;
	}

	printf("-----------------\n");

}

int vartable_hasitem(VariableTable* v1){
	VariableTable* ptr=v1;

	while(ptr->next!=NULL){
		if(ptr->next->variable->name[0]!='%'){
			//匿名変数(アンダーバー・%anonymousXX)でない場合
			return 1;
		}

		ptr=ptr->next;
	}

	return 0;
}

void term_show(Term* t){
	switch(t->tag){
	case TERM_INTEGER:
		printf("%d",t->value.integer);
		break;
	case TERM_STRUCTURE:
		structure_show(*(t->value.structure));
		break;
	case TERM_UNBOUND:
		printf("<unbound>");
		break;
	case TERM_VARIABLE:
		printf("%s",t->value.variable->name);
		break;
	case TERM_PPTERM:
		term_show(t->value.ppterm->termptr);
		break;
	default:
		printf("<unknown>");
		break;
	}
	//printf("  [address=%p]",t);
}

void list_show(Structure s){
	//functorがドットであると仮定
	Term* second_arg=s.arguments->next->next->term;
	term_show(s.arguments->next->term);
	second_arg=term_remove_ppterm(second_arg);
	if(second_arg->tag==TERM_STRUCTURE && strcmp(second_arg->value.structure->functor->name,".")==0){
		//cdrもリスト
		printf(","); list_show(*(second_arg->value.structure));
	}else if(second_arg->tag==TERM_STRUCTURE && strcmp(second_arg->value.structure->functor->name,"[]")==0 && structure_arity(second_arg->value.structure)==0){
		// [] が来た　→　リスト終端
		return;
	}else{
		//printf("<<tag:%d name:>>\n",second_arg.tag);
		printf("|"); term_show(s.arguments->next->next->term);
	}

}

void structure_show(Structure s){
	TermList* ptr=s.arguments;

	if(strcmp(s.functor->name,".")==0){
		//リスト
		printf("["); list_show(s);printf("]");
		return;
	}

	printf("%s",s.functor->name);
	if(ptr->next==NULL){
		//arity=0
		return;
	}

	printf("(");
	while(ptr->next!=NULL){
		term_show(ptr->next->term);
		ptr=ptr->next;
		if(ptr->next!=NULL){printf(", ");}
	}
	printf(")");
}

VariableTable* vartable_findvar(VariableTable* vl,Variable var){
	VariableTable* ptr=vl;

	while(ptr->next!=NULL){
		if(ptr->next->variable==var){
			return ptr->next;
		}
		ptr=ptr->next;
	}
	printf("variable: not found! (%s)\n",var->name);

	return NULL;
}



void htstack_pushnew(HTStack *hts){
	gcmemstack_pushnew(&GCMEMSTACK);
	HTStack* ptr=hts;
	//printf("{{pushnew}}\n");

	while(ptr->next!=NULL){
		ptr=ptr->next;
	}
	ptr->next=gc_malloc(sizeof(HTStack),F_HTSTACK,&GCMEMSTACK);
	ptr->next->htable=gc_malloc(sizeof(HistoryTable),F_HISTORYTABLE,&GCMEMSTACK);
	ptr->next->htable->next=NULL;
	ptr->next->next=NULL;
	gcmemstack_pop(&GCMEMSTACK);
	return;
}

void htstack_push(HTStack *hts,HistoryTable* htable){
	gcmemstack_pushnew(&GCMEMSTACK);
	HTStack* ptr=hts;
	//printf("{{push}}\n");

	while(ptr->next!=NULL){
		ptr=ptr->next;
	}

	ptr->next=gc_malloc(sizeof(HTStack),F_HTSTACK,&GCMEMSTACK);
	ptr->next->htable=htable;
	ptr->next->next=NULL;

	gcmemstack_pop(&GCMEMSTACK);
	return;
}

void htstack_pop(HTStack *hts){
	//printf("{{{pop called}}}\n");
	HTStack* ptr=hts;
	HTStack* prev=NULL;
	int cnt=0;
	while(ptr->next!=NULL){
		cnt++;
		prev=ptr;
		ptr=ptr->next;
	}

	//printf("count=%d\n",cnt);

	if(hts->next==NULL){ return;}

	//巻き戻し
	HistoryTable* hptr=ptr->htable;
	while(hptr->next!=NULL){
		if(hptr->next->prev==NULL){
			hptr->next->pterm->tag=TERM_UNBOUND;
			//printf("1 unbind\n");
		}else{
			hptr->next->ppterm->termptr=hptr->next->prev;
			//printf("redo:pointer\n");
		}

		hptr=hptr->next;
	}

	if(prev!=NULL){
		prev->next=NULL;
	}

	return;
}

HistoryTable* htstack_toptable(HTStack hts){
	HTStack* ptr=&hts;
	while(ptr->next!=NULL){
		ptr=ptr->next;
	}

	return ptr->htable;
}


int structure_arity(Structure* s){
	int arity=0;
	TermList* t_ptr=s->arguments;
	while(t_ptr->next!=NULL){
		arity++;
		t_ptr=t_ptr->next;
	}
	return arity;
}
