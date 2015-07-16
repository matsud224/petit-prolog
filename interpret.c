#include <stdio.h>
#include <malloc.h>
#include "header.h"


HTStack GlobalStack;

void interpret(FILE* fp){
    Program p = parse_program(fp);
    Program* ptr=&p;

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
}

void interpret_clause(Clause clause){
	clause.arity=structure_arity(clause.head);

	//節の登録を行う
	SymbolTable* syminfo=clause.head.functor;
	ClauseList* cl_ptr=&syminfo->clause_list;
	while(cl_ptr->next!=NULL){
		cl_ptr=cl_ptr->next;
	}
	cl_ptr->next=malloc(sizeof(ClauseList));
	cl_ptr->next->clause=malloc(sizeof(Clause));
	cl_ptr->next->next=NULL;
	*(cl_ptr->next->clause)=clause;
}

//Boxのメモリを確保し、それへのポインタを返します
Box* box_get(){
	Box* b=malloc(sizeof(Box));
	b->failure=NULL;
	b->success=NULL;
	b->is_begin=0;
	b->is_end=0;
	b->is_failed=0;
	b->selected_clause=NULL;

	return b;
}

void interpret_question(Question question){
	GlobalStack.next=NULL;
	//箱の準備
	Box* beginbox=box_get(); beginbox->is_begin=1;
	Box* endbox=box_get(); endbox->is_end=1;
	Box* prev=beginbox;
	Box* current;

	VariableTable* vartable=malloc(sizeof(VariableTable));
	vartable->next=NULL;
	vartable_from_question(vartable,question);

    StructureList* ptr;
    for(ptr=&(question.body);ptr->next!=NULL;ptr=ptr->next){
		current=box_get();
		current->structure=ptr->next->structure;
		current->selected_clause=&(current->structure.functor->clause_list);
		current->vartable=vartable;
		prev->success=current;
		current->failure=prev;
		prev=current;
    }
    prev->success=endbox;
	endbox->failure=prev;
	endbox->vartable=vartable;

	execute(beginbox->success);
}

void execute(Box* current){
retry:

	while(!(current->is_end)){
		VariableTable* callee_new_vartable;

		//printf("start\n");
		//structure_show(current->structure);

		next_clause(current,&callee_new_vartable);

		if(current->is_failed){
			//printf("fail\n");
			//リセット
			current->is_failed=0;
			current->selected_clause=&(current->structure.functor->clause_list);
			current=current->failure;

			htstack_pop(&GlobalStack);

			if(current->is_begin){
				printf("\nno.\n");
				return;
			}
		}else{
			//printf("success\n");
			if(current->selected_clause->clause->body.next!=NULL){
				//printf("subgoals*prepare\n");
				//サブゴール有り
                //箱の準備
				Box* beginbox=current;
				Box* endbox=current->success;
				Box* prev=beginbox;
				Box* curr_box;

				StructureList* ptr;
				for(ptr=&(current->selected_clause->clause->body);ptr->next!=NULL;ptr=ptr->next){
					curr_box=box_get();
					curr_box->structure=ptr->next->structure;
					curr_box->selected_clause=&(curr_box->structure.functor->clause_list);
					curr_box->vartable=callee_new_vartable;
					prev->success=curr_box;
					curr_box->failure=prev;
					prev=curr_box;
				}

				prev->success=endbox;
				endbox->failure=prev;

				//printf("prepared.\n");
			}

			current=current->success;
		}
	}

	//解を発見
	if(current->vartable->next!=NULL){
		//変数テーブルに１つ以上あったら
		vartable_show(*(current->vartable));
		printf("\n this? (y/n) : ");
		char ans='\0';
		while(ans!='y' && ans!='n'){
			ans=getc(stdin);
		}
		if(ans=='y'){
			printf("\nyes.\n"); return;
		}else{
			current=current->failure;

			htstack_pop(&GlobalStack);

			if(current->is_begin){
				printf("\nno.\n");
				return;
			}
			goto retry;
		}
	}else{
		printf("\nyes.\n");
	}
}


void next_clause(Box* box,VariableTable** vt_callee){
	//すでに失敗しているとき
	if(box->is_failed){return;}

	if(!(box->selected_clause)){box->is_failed=1;return;}

	ClauseList* cl_ptr;

	int arity=structure_arity(box->structure);

    for(cl_ptr=box->selected_clause;cl_ptr->next!=NULL;cl_ptr=cl_ptr->next){
		if(arity==cl_ptr->next->clause->arity){
			//printf("\nnext_clause*try\n");
			htstack_pushnew(&GlobalStack);
			HistoryTable* history=htstack_toptable(GlobalStack);

			VariableTable* new_vartable=malloc(sizeof(VariableTable));
			new_vartable->next=NULL;
			vartable_from_clause(new_vartable,*(cl_ptr->next->clause));

			Structure* portable1=structure_to_portable(&(box->structure),*(box->vartable));
			Structure* portable2=structure_to_portable(&(cl_ptr->next->clause->head),*new_vartable);

			/*
			printf("unify start-----------\n");
			structure_show(*portable1);
			printf("\n");
			structure_show(*portable2);
			printf("unify end-------------\n");
			*/

			if(structure_unify(*portable1,*portable2,box->vartable,new_vartable,history)){
				//printf("\nnext_clause*success\n");
				box->selected_clause=cl_ptr->next;
				*vt_callee=new_vartable;
				return;
			}else{
				//printf("\nnext_clause*fail\n");
				htstack_pop(&GlobalStack);
			}
		}

    }

	box->is_failed=1;

	return;
}

void vartable_from_clause(VariableTable* vt,Clause c){
	StructureList* sl_ptr=&(c.body);

	vartable_from_structure(vt,c.head);

	while(sl_ptr->next!=NULL){
		vartable_from_structure(vt,sl_ptr->next->structure);

		sl_ptr=sl_ptr->next;
	}

}

void vartable_from_question(VariableTable* vt,Question q){
	StructureList* sl_ptr=&(q.body);

	while(sl_ptr->next!=NULL){
		vartable_from_structure(vt,sl_ptr->next->structure);

		sl_ptr=sl_ptr->next;
	}

}

void vartable_from_structure(VariableTable* vt,Structure s){
	//重複は考えない（後でまとめて削除する）
	TermList* tl_ptr=&(s.arguments);

	while(tl_ptr->next!=NULL){
		if(tl_ptr->next->term.tag==TERM_STRUCTURE){
			vartable_from_structure(vt,*(tl_ptr->next->term.value.structure));
		}else if(tl_ptr->next->term.tag==TERM_VARIABLE){
			vartable_addvar(vt,tl_ptr->next->term.value.variable);
		}

		tl_ptr=tl_ptr->next;
	}
}



int structure_unify(Structure s1,Structure s2,VariableTable* v1,VariableTable* v2,HistoryTable* h){
	TermList* s1_ptr;
	TermList* s2_ptr;

	if(s1.functor!=s2.functor){
		return 0;
	}

	if(structure_arity(s1)!=structure_arity(s2)){
		return 0;
	}

	s1_ptr=&(s1.arguments); s2_ptr=&(s2.arguments);

	while(s1_ptr->next!=NULL){

		if(!term_unify(&(s1_ptr->next->term),&(s2_ptr->next->term),v1,v2,h)){
			return 0;
		}

		s1_ptr=s1_ptr->next;
		s2_ptr=s2_ptr->next;
	}

	return 1;
}

Term* term_remove_ppterm(Term* t){
	if(t->tag==TERM_PPTERM){
		return term_remove_ppterm(*(t->value.ppterm));
	}else{
		return t;
	}
}

int term_unify(Term* caller,Term* callee,VariableTable* v1,VariableTable* v2,HistoryTable* h){
	if(caller->tag==TERM_INTEGER && callee->tag==TERM_INTEGER){
		return caller->value.integer==callee->value.integer;
	}else if(caller->tag==TERM_STRUCTURE && callee->tag==TERM_STRUCTURE){
		return structure_unify(*(caller->value.structure),*(callee->value.structure),v1,v2,h);
	}else if(caller->tag==TERM_INTEGER && callee->tag==TERM_STRUCTURE){
		return 0;
	}else if(caller->tag==TERM_STRUCTURE && callee->tag==TERM_INTEGER){
		return 0;
	}else if(caller->tag==TERM_PPTERM && callee->tag==TERM_PPTERM
			 && (*(*(caller->value.ppterm))).tag==TERM_UNBOUND && (*(*(callee->value.ppterm))).tag==TERM_UNBOUND){

		htable_addforward(h,callee->value.ppterm,*(callee->value.ppterm));
		*(callee->value.ppterm)=*(caller->value.ppterm);
		return 1;

	}else if(caller->tag==TERM_PPTERM && (*(*(caller->value.ppterm))).tag==TERM_UNBOUND){
		*(*caller->value.ppterm)=*term_remove_ppterm(callee);
		htable_add(h,*(caller->value.ppterm));
		return 1;
	}else if(callee->tag==TERM_PPTERM && (*(*(callee->value.ppterm))).tag==TERM_UNBOUND){
		*(*callee->value.ppterm)=*term_remove_ppterm(caller);
		htable_add(h,*(callee->value.ppterm));
		return 1;
	}else{
		return term_unify(term_remove_ppterm(caller),term_remove_ppterm(callee),v1,v2,h);
	}

	return 0;
}


Structure* structure_to_portable(Structure* s,VariableTable vt){
	Structure* result=malloc(sizeof(Structure));
	TermList* ptr;
	TermList* res_ptr;

	result->functor=s->functor;
	result->arguments.next=NULL;

	ptr=&(s->arguments);
	res_ptr=&(result->arguments);
	while(ptr->next!=NULL){
		res_ptr->next=malloc(sizeof(TermList));
		res_ptr->next->term=term_to_portable(&(ptr->next->term),vt);

		ptr=ptr->next;
		res_ptr=res_ptr->next;
	}
	res_ptr->next=NULL;
	return result;
}

Term term_to_portable(Term* t,VariableTable vt){
	Term result;
	switch(t->tag){
	case TERM_INTEGER:
		return *t;
	case TERM_STRUCTURE:
		result.tag=TERM_STRUCTURE;
		result.value.structure=structure_to_portable(t->value.structure,vt);
		return result;
	case TERM_VARIABLE:
		result.tag=TERM_PPTERM;
		result.value.ppterm=vartable_findvar(vt,t->value.variable);
		return result;
	case TERM_UNBOUND:
		error("unexpected case.");
		break;
	default:
		error("invalid term.");
		break;
	}
}

