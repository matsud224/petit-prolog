#include <stdio.h>
#include "header.h"


HTStack GlobalStack;
Box* CURRENT_BEGINBOX;

void interpret(FILE* fp){
	gcmemstack_pushnew(&GCMEMSTACK);

	//printf("beforesize = %d\n",gcmemlist_size(gcmemstack_top(&GCMEMSTACK)));
    Program* p = parse_program(fp);
    //printf("aftersize = %d\n",gcmemlist_size(gcmemstack_top(&GCMEMSTACK)));
    Program* ptr=p;

    while(ptr->next!=NULL){
        switch(ptr->next->tag){
        case PROG_CLAUSE:
			interpret_clause(ptr->next->item.clause);
			break;
		case PROG_QUESTION:
			interpret_question(ptr->next->item.question);
			break;
		}

		ptr=ptr->next;
    }
    gcmemstack_pop(&GCMEMSTACK);
}

void interpret_clause(Clause* clause){
	gcmemstack_pushnew(&GCMEMSTACK);
	clause->arity=structure_arity(clause->head);
	//節の登録を行う
	SymbolTable* syminfo=clause->head->functor;
	ClauseList* cl_ptr=syminfo->clause_list;
	while(cl_ptr->next!=NULL){
		cl_ptr=cl_ptr->next;
	}
	cl_ptr->next=gc_malloc(sizeof(ClauseList),F_CLAUSELIST,&GCMEMSTACK);
	cl_ptr->next->next=NULL;
	cl_ptr->next->clause=clause;

	gcmemstack_pop(&GCMEMSTACK);
}

//Boxのメモリを確保し、それへのポインタを返します
Box* box_get(){
	gcmemstack_pushnew(&GCMEMSTACK);
	Box* b=gc_malloc(sizeof(Box),F_BOX,&GCMEMSTACK);
	b->failure=NULL;
	b->success=NULL;
	b->is_begin=0;
	b->is_end=0;
	b->is_failed=0;
	b->selected_clause=NULL;
	gcmemstack_returnptr(b,&GCMEMSTACK);
	gcmemstack_pop(&GCMEMSTACK);
	return b;
}

void interpret_question(Question* question){
	gcmemstack_pushnew(&GCMEMSTACK);
	GlobalStack.next=NULL;


	//箱の準備
	Box* beginbox=box_get(); beginbox->is_begin=1;

	CURRENT_BEGINBOX=beginbox;

	Box* endbox=box_get(); endbox->is_end=1;
	Box* prev=beginbox;
	Box* current;

	VariableTable* vartable=gc_malloc(sizeof(VariableTable),F_VARIABLETABLE,&GCMEMSTACK);
	vartable->next=NULL;
	vartable_from_question(vartable,question);

    StructureList* ptr;
    for(ptr=question->body;ptr->next!=NULL;ptr=ptr->next){
		current=box_get();
		current->structure=ptr->next->structure;
		current->selected_clause=current->structure->functor->clause_list;
		current->vartable=vartable;
		prev->success=current;
		current->failure=prev;
		prev=current;
    }
    prev->success=endbox;
	endbox->failure=prev;
	endbox->vartable=vartable;

	execute(beginbox->success);

	CURRENT_BEGINBOX=NULL;
	GlobalStack.next=NULL;

	gcmemstack_pop(&GCMEMSTACK);
}

void execute(Box* current){
	gcmemstack_pushnew(&GCMEMSTACK);
retry:

	while(!(current->is_end)){
		VariableTable* callee_new_vartable;

		if(current->is_failed){
			goto fail_process;
		}

		if(current->structure->functor==sym_get("!") && structure_arity(current->structure)==0){
			//カット！
			Box* boxptr=current;
			while(1){
				boxptr->is_failed=1;

				if(boxptr->failure==NULL || boxptr->failure->vartable!=boxptr->vartable){
					//vartableの変わり目
					if(boxptr->failure!=NULL){
						boxptr->failure->is_failed=1;
					}
					break;
				}else{
					boxptr=boxptr->failure;
				}
			}

			current=current->success;
			continue;
		}

		next_clause(current,&callee_new_vartable);

		if(current->is_failed){
fail_process:
			//リセット
			current->is_failed=0;
			current->selected_clause=current->structure->functor->clause_list;
			current=current->failure;

			htstack_pop(&GlobalStack);

			if(current->is_begin){
				printf("\nno.\n");
				gcmemstack_pop(&GCMEMSTACK);
				return;
			}
		}else{
			if(current->selected_clause->clause->body->next!=NULL){
				//サブゴール有り
                //箱の準備
				Box* beginbox=current;
				Box* endbox=current->success;
				Box* prev=beginbox;
				Box* curr_box;

				StructureList* ptr;
				for(ptr=current->selected_clause->clause->body;ptr->next!=NULL;ptr=ptr->next){
					curr_box=box_get();
					curr_box->structure=ptr->next->structure;
					curr_box->selected_clause=curr_box->structure->functor->clause_list;
					curr_box->vartable=callee_new_vartable;
					prev->success=curr_box;
					curr_box->failure=prev;
					prev=curr_box;
				}

				prev->success=endbox;
				endbox->failure=prev;
			}

			current=current->success;
		}
	}

	//解を発見
	if(vartable_hasitem(current->vartable)){
		//変数テーブルに１つ以上あったら
		vartable_show(*(current->vartable));
		printf("\n this? (y/n) : ");
		char ans='\0';
		while(ans!='y' && ans!='n'){
			ans=getc(stdin);
		}
		if(ans=='y'){
			printf("\nyes.\n"); gcmemstack_pop(&GCMEMSTACK);return;
		}else{
			current=current->failure;

			htstack_pop(&GlobalStack);

			if(current->is_begin){
				printf("\nno.\n");
				gcmemstack_pop(&GCMEMSTACK);
				return;
			}

			goto retry;
		}
	}else{
		printf("\nyes.\n");
	}

	gcmemstack_pop(&GCMEMSTACK);
}


void next_clause(Box* box,VariableTable** vt_callee){
	gcmemstack_pushnew(&GCMEMSTACK);

	//すでに失敗しているとき
	if(box->is_failed){gcmemstack_pop(&GCMEMSTACK);return;}

	if(!(box->selected_clause)){box->is_failed=1;gcmemstack_pop(&GCMEMSTACK);return;}

	ClauseList* cl_ptr;

	int arity=structure_arity(box->structure);

    for(cl_ptr=box->selected_clause;cl_ptr->next!=NULL;cl_ptr=cl_ptr->next){
		if(arity==cl_ptr->next->clause->arity){
			htstack_pushnew(&GlobalStack);
			HistoryTable* history=htstack_toptable(GlobalStack);

			VariableTable* new_vartable=gc_malloc(sizeof(VariableTable),F_VARIABLETABLE,&GCMEMSTACK);
			new_vartable->next=NULL;
			vartable_from_clause(new_vartable,cl_ptr->next->clause);

			Structure* portable1=structure_to_portable(box->structure,box->vartable);
			Structure* portable2=structure_to_portable(cl_ptr->next->clause->head,new_vartable);

			if(structure_unify(portable1,portable2,box->vartable,new_vartable,history)){
				box->selected_clause=cl_ptr->next;
				*vt_callee=new_vartable;
				gcmemstack_pop(&GCMEMSTACK);
				return;
			}else{
				htstack_pop(&GlobalStack);
			}
		}

    }

	box->is_failed=1;

	gcmemstack_pop(&GCMEMSTACK);

	return;
}

void vartable_from_clause(VariableTable* vt,Clause* c){
	gcmemstack_pushnew(&GCMEMSTACK);
	StructureList* sl_ptr=c->body;

	vartable_from_structure(vt,c->head);

	while(sl_ptr->next!=NULL){
		vartable_from_structure(vt,sl_ptr->next->structure);

		sl_ptr=sl_ptr->next;
	}
	gcmemstack_pop(&GCMEMSTACK);
}

void vartable_from_question(VariableTable* vt,Question* q){
	gcmemstack_pushnew(&GCMEMSTACK);
	StructureList* sl_ptr=q->body;

	while(sl_ptr->next!=NULL){
		vartable_from_structure(vt,sl_ptr->next->structure);

		sl_ptr=sl_ptr->next;
	}
	gcmemstack_pop(&GCMEMSTACK);
}

void vartable_from_structure(VariableTable* vt,Structure* s){
	gcmemstack_pushnew(&GCMEMSTACK);
	//重複は考えない（後でまとめて削除する）
	TermList* tl_ptr=s->arguments;

	while(tl_ptr->next!=NULL){
		if(tl_ptr->next->term->tag==TERM_STRUCTURE){
			vartable_from_structure(vt,tl_ptr->next->term->value.structure);
		}else if(tl_ptr->next->term->tag==TERM_VARIABLE){
			vartable_addvar(vt,tl_ptr->next->term->value.variable);
		}

		tl_ptr=tl_ptr->next;
	}
	gcmemstack_pop(&GCMEMSTACK);
}



int structure_unify(Structure* s1,Structure* s2,VariableTable* v1,VariableTable* v2,HistoryTable* h){
	gcmemstack_pushnew(&GCMEMSTACK);
	TermList* s1_ptr;
	TermList* s2_ptr;

	if(s1->functor!=s2->functor){
		gcmemstack_pop(&GCMEMSTACK);
		return 0;
	}

	if(structure_arity(s1)!=structure_arity(s2)){
		gcmemstack_pop(&GCMEMSTACK);
		return 0;
	}

	s1_ptr=s1->arguments; s2_ptr=s2->arguments;

	while(s1_ptr->next!=NULL){

		if(!term_unify(s1_ptr->next->term,s2_ptr->next->term,v1,v2,h)){
			gcmemstack_pop(&GCMEMSTACK);
			return 0;
		}

		s1_ptr=s1_ptr->next;
		s2_ptr=s2_ptr->next;
	}

	gcmemstack_pop(&GCMEMSTACK);
	return 1;
}

Term* term_remove_ppterm(Term* t){
	gcmemstack_pushnew(&GCMEMSTACK);
	if(t->tag==TERM_PPTERM){
		gcmemstack_pop(&GCMEMSTACK);
		return term_remove_ppterm(t->value.ppterm->termptr);
	}else{
		gcmemstack_pop(&GCMEMSTACK);
		return t;
	}

}

int term_unify(Term* caller,Term* callee,VariableTable* v1,VariableTable* v2,HistoryTable* h){
	gcmemstack_pushnew(&GCMEMSTACK);
	if(caller->tag==TERM_INTEGER && callee->tag==TERM_INTEGER){
		gcmemstack_pop(&GCMEMSTACK);
		return caller->value.integer==callee->value.integer;
	}else if(caller->tag==TERM_STRUCTURE && callee->tag==TERM_STRUCTURE){
		gcmemstack_pop(&GCMEMSTACK);
		return structure_unify(caller->value.structure,callee->value.structure,v1,v2,h);
	}else if(caller->tag==TERM_INTEGER && callee->tag==TERM_STRUCTURE){
		gcmemstack_pop(&GCMEMSTACK);
		return 0;
	}else if(caller->tag==TERM_STRUCTURE && callee->tag==TERM_INTEGER){
		gcmemstack_pop(&GCMEMSTACK);
		return 0;
	}else if(caller->tag==TERM_PPTERM && callee->tag==TERM_PPTERM
			 && caller->value.ppterm->termptr->tag==TERM_UNBOUND && callee->value.ppterm->termptr->tag==TERM_UNBOUND){

		htable_addforward(h,callee->value.ppterm,callee->value.ppterm->termptr);
		callee->value.ppterm->termptr=caller->value.ppterm->termptr;
		gcmemstack_pop(&GCMEMSTACK);
		return 1;

	}else if(caller->tag==TERM_PPTERM && caller->value.ppterm->termptr->tag==TERM_UNBOUND){
		*(caller->value.ppterm->termptr)=*term_remove_ppterm(callee);
		htable_add(h,caller->value.ppterm->termptr);
		gcmemstack_pop(&GCMEMSTACK);
		return 1;
	}else if(callee->tag==TERM_PPTERM && callee->value.ppterm->termptr->tag==TERM_UNBOUND){
		*(callee->value.ppterm->termptr)=*term_remove_ppterm(caller);
		htable_add(h,callee->value.ppterm->termptr);
		gcmemstack_pop(&GCMEMSTACK);
		return 1;
	}else{
		gcmemstack_pop(&GCMEMSTACK);
		return term_unify(term_remove_ppterm(caller),term_remove_ppterm(callee),v1,v2,h);
	}

	gcmemstack_pop(&GCMEMSTACK);
	return 0;
}


Structure* structure_to_portable(Structure* s,VariableTable* vt){
	gcmemstack_pushnew(&GCMEMSTACK);
	Structure* result=gc_malloc(sizeof(Structure),F_STRUCTURE,&GCMEMSTACK);
	TermList* ptr;
	TermList* res_ptr;

	result->functor=s->functor;
	result->arguments=gc_malloc(sizeof(TermList),F_TERMLIST,&GCMEMSTACK);
	result->arguments->next=NULL;

	ptr=s->arguments;
	res_ptr=result->arguments;
	while(ptr->next!=NULL){
		res_ptr->next=gc_malloc(sizeof(TermList),F_TERMLIST,&GCMEMSTACK);
		res_ptr->next->term=term_to_portable(ptr->next->term,vt);

		ptr=ptr->next;
		res_ptr=res_ptr->next;
	}
	res_ptr->next=NULL;
	gcmemstack_returnptr(result,&GCMEMSTACK);
	gcmemstack_pop(&GCMEMSTACK);
	return result;
}

Term* term_to_portable(Term* t,VariableTable* vt){
	gcmemstack_pushnew(&GCMEMSTACK);

	Term* result=gc_malloc(sizeof(Term),F_TERM,&GCMEMSTACK);
	switch(t->tag){
	case TERM_INTEGER:
		gcmemstack_pop(&GCMEMSTACK);
		return t;
	case TERM_STRUCTURE:
		result->tag=TERM_STRUCTURE;
		result->value.structure=structure_to_portable(t->value.structure,vt);
		gcmemstack_returnptr(result,&GCMEMSTACK);
		gcmemstack_pop(&GCMEMSTACK);
		return result;
	case TERM_VARIABLE:
		result->tag=TERM_PPTERM;
		result->value.ppterm=vartable_findvar(vt,t->value.variable);
		gcmemstack_returnptr(result,&GCMEMSTACK);
		gcmemstack_pop(&GCMEMSTACK);
		return result;
	case TERM_UNBOUND:
		error("unexpected case.");
		break;
	default:
		error("invalid term.");
		break;
	}
	gcmemstack_pop(&GCMEMSTACK);
}

