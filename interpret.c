#include <stdio.h>
#include <malloc.h>
#include "header.h"




void interpret(FILE* fp){
    Program p = parse_program(fp);
    Program* ptr=&p;
	VariableTable temp;
	VTStack stack; stack.next=NULL;
    while(ptr->next!=NULL){
        switch(ptr->next->tag){
        case PROG_CLAUSE:
			interpret_clause(ptr->next->item.clause);

/*
			printf("-------test-------\n");
			temp=vartable_from_clause(ptr->next->item.clause);
			printf("*push*\n");
			vtstack_push(&stack,vartable_copy(temp));
			vartable_add(&temp,sym_get("PROLOG"));
			vtstack_push(&stack,temp);
			printf("*duplicate*\n");
			vtstack_duplicate(&stack);
			vtstack_pop(&stack);
			vtstack_pop(&stack);
			printf("*size:%d*",vtstack_size(stack));
			printf("*show*\n");
			vartable_show(*vtstack_toptable(stack));
			printf("-------end test-------\n");
*/
			break;
		case PROG_QUESTION:
			interpret_question(ptr->next->item.question);
/*
			printf("-------test-------\n");
			vartable_show(vartable_from_question(ptr->next->item.question));
			printf("-------end test-------\n");
	*/
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
	b->vt_stack.next=NULL;
	b->selected_clause=NULL;

	return b;
}

void interpret_question(Question question){
	//箱の準備
	Box* beginbox=box_get(); beginbox->is_begin=1;
	Box* endbox=box_get(); endbox->is_end=1;
	Box* prev=beginbox;
	Box* current;
	VTStack stack; stack.next=NULL;

	vtstack_push(&stack,vartable_from_question(question));

    StructureList* ptr;
    for(ptr=&(question.body);ptr->next!=NULL;ptr=ptr->next){
		current=box_get();
		current->vt_stack=stack;
		current->structure=ptr->next->structure;
		current->selected_clause=&(current->structure.functor->clause_list);
		prev->success=current;
		current->failure=prev;
		prev=current;
    }
    prev->success=endbox;
	endbox->failure=prev;

	endbox->vt_stack=stack;

	//vtstack_push(&(beginbox->success->vt_stack),vartable_from_question(question));

	//printf("-start-\n");
	execute(beginbox->success);
}

void execute(Box* current){
	VariableTable vt_caller,vt_callee;
retry:
	while(!(current->is_end)){
		vt_caller.next=NULL; vt_callee.next=NULL;


		//printf("-execute-\n");
		next_clause(current,&vt_caller,&vt_callee);
		//vartable_show(*vtstack_toptable(current->vt_stack));
		//printf("-clause selected-\n");
		if(current->is_failed){
			//printf("-failed-\n");
			//vtstack_pop(&(current->vt_stack));

			//リセット
			current->is_failed=0;
			current->selected_clause=&(current->structure.functor->clause_list);
			current=current->failure;

			vtstack_pop(&(current->vt_stack));

			if(current->is_begin){
				printf("\nno.\n");
				return;
			}
		}else{
			if(current->selected_clause->clause->body.next!=NULL){
				//printf("-subgoals-\n");
				//サブゴール有り
                //箱の準備
				Box* beginbox=current;
				Box* endbox=current->success;
				Box* prev=beginbox;
				Box* curr_box;
				VTStack stack; stack.next=NULL;
				vtstack_push(&stack,vt_callee);
				vtstack_boundcheck_top(stack);

				StructureList* ptr;
				for(ptr=&(current->selected_clause->clause->body);ptr->next!=NULL;ptr=ptr->next){
					curr_box=box_get();
					curr_box->vt_stack=stack;
					curr_box->structure=ptr->next->structure;
					curr_box->selected_clause=&(curr_box->structure.functor->clause_list);
					prev->success=curr_box;
					curr_box->failure=prev;
					prev=curr_box;
				}

				prev->success=endbox;
				endbox->failure=prev;
			}else{
				//printf("-no subgoals-\n");

			}

			vtstack_push(&(current->vt_stack),vt_caller);
			vtstack_boundcheck_top(current->vt_stack);

			current=current->success;
		}


	}

	//解を発見
	if(vtstack_toptable(current->vt_stack)->next!=NULL){
		//変数テーブルに１つ以上あったら
		vartable_show(*vtstack_toptable(current->vt_stack));
		printf("\n this? (y/n) : ");
		char ans='\0';
		while(ans!='y' && ans!='n'){
			ans=getc(stdin);
		}
		if(ans=='y'){
			printf("\nyes.\n"); return;
		}else{

			current=current->failure;

			vtstack_pop(&(current->vt_stack));

			//printf("size: %d .",vtstack_size(current->vt_stack));
			//vartable_show(current->vt_stack.next->next->vartable);
			//vartable_show(current->vt_stack.next->vartable);
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


void next_clause(Box* box,VariableTable* vt_caller_ret,VariableTable* vt_callee_ret){
	ClauseList* cl_ptr;

	int arity=structure_arity(box->structure);

	//すでに失敗しているとき
	if(box->is_failed){return;}

	if(!(box->selected_clause)){box->is_failed=1;return;}

    for(cl_ptr=box->selected_clause;cl_ptr->next!=NULL;cl_ptr=cl_ptr->next){
		//printf("-try-\n");
		if(arity==cl_ptr->next->clause->arity){
			//printf("unify start---");structure_show(box->structure);printf(" and ");structure_show(cl_ptr->next->clause->head);printf("\n");
			VariableTable vt_caller=vartable_copy(*vtstack_toptable(box->vt_stack));
			VariableTable vt_callee=vartable_from_clause(*(cl_ptr->next->clause));
			//printf("-unification start-\n");
			//vartable_show(vt_caller);
			//vartable_show(vt_callee);

			vartable_boundcheck(vt_caller);

			if(structure_unify(vt_caller,box->structure,vt_callee,cl_ptr->next->clause->head)){
				//printf("success.\n");
				box->selected_clause=cl_ptr->next;
				//printf("-unification success-\n");

				*vt_caller_ret=vt_caller;
				*vt_callee_ret=vt_callee;

				//vartable_show(*vtstack_toptable(box->vt_stack));
				//printf("^^^^^^^^^^^^^^^^^^^^");
				//vartable_show(vt_caller);
				return;
			}/*else{
				VariableTable* ptr1;
				for(ptr1=&vt_caller;ptr1->next!=NULL;ptr1=ptr1->next){
					if(ptr1->next->value.tag==TERM_POINTER && ptr1->next->value.value.pointer->tag!=TERM_UNBOUND && ptr1->next->value.ref_bound==0){
						printf("\n\n***unbind***\n\n");
						ptr1->next->value.value.pointer->tag=TERM_UNBOUND;
					}
				}

				//vartable_show(*vtstack_toptable(box->vt_stack));
				printf("^^^^^^^^^^^^^^^^^^^^");
				vartable_show(vt_caller);
				vartable_show(vt_callee);
			}*/
			//printf("fail.\n");
		}
    }

	box->is_failed=1;

	return;
}

VariableTable vartable_from_clause(Clause c){
	StructureList* sl_ptr=&(c.body);
	VariableTable vl; vl.next=NULL;

	vartable_concat(&vl,vartable_from_structure(c.head));

	while(sl_ptr->next!=NULL){
		vartable_concat(&vl,vartable_from_structure(sl_ptr->next->structure));

		sl_ptr=sl_ptr->next;
	}

	vartable_unique(vl);

	return vl;
}

VariableTable vartable_from_question(Question q){
	StructureList* sl_ptr=&(q.body);
	VariableTable vl; vl.next=NULL;
	while(sl_ptr->next!=NULL){
		vartable_concat(&vl,vartable_from_structure(sl_ptr->next->structure));

		sl_ptr=sl_ptr->next;
	}

	vartable_unique(vl);

	return vl;
}

VariableTable vartable_from_structure(Structure s){
	//重複は考えない（後でまとめて削除する）
	TermList* tl_ptr=&(s.arguments);
	VariableTable vl; vl.next=NULL;
	while(tl_ptr->next!=NULL){
		if(tl_ptr->next->term.tag==TERM_STRUCTURE){
			vartable_concat(&vl,vartable_from_structure(*(tl_ptr->next->term.value.structure)));
		}else if(tl_ptr->next->term.tag==TERM_VARIABLE){
			tl_ptr->next->term.ref_bound=0;
			vartable_add(&vl,tl_ptr->next->term.value.variable);
		}

		tl_ptr=tl_ptr->next;
	}

	vartable_unique(vl);

	return vl;
}



int structure_unify(VariableTable v1,Structure s1,VariableTable v2,Structure s2){
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

		if(!term_unify(v1,&(s1_ptr->next->term),v2,&(s2_ptr->next->term))){
			//printf("-term unify failed-\n");
			return 0;
		}

		//printf("-term unify success-\n");
		s1_ptr=s1_ptr->next;
		s2_ptr=s2_ptr->next;
	}

	return 1;
}

int term_unify(VariableTable vl_caller,Term* caller,VariableTable vl_callee,Term* callee){
	//printf("-term unify-\n");
	//printf("-tag %d %d -",caller->tag,callee->tag);
	if(caller->tag==TERM_INTEGER && callee->tag==TERM_INTEGER){
		return caller->value.integer==callee->value.integer;
	}else if(caller->tag==TERM_STRUCTURE && callee->tag==TERM_STRUCTURE){
		return structure_unify(vl_caller,*(caller->value.structure),vl_callee,*(callee->value.structure));
	}else if(caller->tag==TERM_VARIABLE && callee->tag==TERM_VARIABLE){
		return term_unify(vl_caller,vartable_find(vl_caller,caller->value.variable),vl_callee,vartable_find(vl_callee,callee->value.variable));
	}else if(caller->tag==TERM_VARIABLE){
		return term_unify(vl_caller,vartable_find(vl_caller,caller->value.variable),vl_callee,callee);
	}else if(callee->tag==TERM_VARIABLE){
		return term_unify(vl_caller,caller,vl_callee,vartable_find(vl_callee,callee->value.variable));
	}else if(caller->tag==TERM_UNBOUND && callee->tag==TERM_UNBOUND){
		Term* sharedterm=malloc(sizeof(Term));
		sharedterm->tag=TERM_UNBOUND;
		callee->tag=TERM_POINTER;
		callee->value.pointer=sharedterm;
		caller->tag=TERM_POINTER;
		caller->value.pointer=sharedterm;
		return 1;
	}else if(caller->tag==TERM_UNBOUND){
		*caller=*callee;
		return 1;
	}else if(callee->tag==TERM_UNBOUND){
		*callee=*caller;
		return 1;
	}else if(caller->tag==TERM_POINTER && callee->tag==TERM_POINTER){
		return term_unify(vl_caller,caller->value.pointer,vl_callee,callee->value.pointer);
	}else if(caller->tag==TERM_POINTER){
		return term_unify(vl_caller,caller->value.pointer,vl_callee,callee);
	}else if(callee->tag==TERM_POINTER){
		return term_unify(vl_caller,caller,vl_callee,callee->value.pointer);
	}

	return 0;
}
