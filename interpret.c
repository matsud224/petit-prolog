#include <stdio.h>
#include <malloc.h>
#include "header.h"




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
	*(cl_ptr->next->clause)=clause;
}

//Boxのメモリを確保し、それへのポインタを返します
Box* box_get(){
	Box* b=malloc(sizeof(Box));
	b->failure=NULL;
	b->success=NULL;
	b->is_begin=0;
	b->is_end=0;
	b->selected_clause=NULL;

	return b;
}

void interpret_question(Question question){
	//箱の準備
	Box* beginbox=box_get(); beginbox->is_begin=1;
	Box* endbox=box_get(); endbox->is_end=1;
	Box* prev=beginbox;
	Box* current;

    StructureList* ptr;
    for(ptr=&(question.body);ptr->next!=NULL;ptr=ptr->next){
		current=box_get();
		prev->success=current;
		current->failure=prev;
		prev=current;
    }
	endbox->failure=prev;

	execute(beginbox->success);
}

void execute(Box* current){
	while(!(current->is_end)){
		next_clause(current);
		if(current->is_failed){
			current=current->failure;
		}else{
			current=current->success;
		}

		if(current->is_begin){
			printf("\nno.\n");
			return;
		}
	}

	printf("\nyes.\n");
}

void next_clause(Box* box){
	ClauseList* cl_ptr;
	int arity=structure_arity(box->structure);

	//すでに失敗しているとき
	if(box->is_failed){return;}

    for(cl_ptr=box->selected_clause;cl_ptr->next!=NULL;cl_ptr=cl_ptr->next){
		if(arity==cl_ptr->next->clause->arity){
			VariableTable vt_caller=vartable_from_clause(*(cl_ptr->next->clause));
			VariableTable vt_callee=vartable_from_structure(box->structure);

			if(structure_unify_test(vt_caller,cl_ptr->next->clause->head,vt_callee,box->structure)){
				box->selected_clause=cl_ptr->next;
				return;
			}
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

	return vl;
}

VariableTable vartable_from_question(Question q){
	StructureList *sl_ptr=&(q.body);
	VariableTable vl; vl.next=NULL;
	while(sl_ptr->next!=NULL){
		vartable_concat(&vl,vartable_from_structure(sl_ptr->next->structure));

		sl_ptr=sl_ptr->next;
	}

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
			vartable_add(&vl,tl_ptr->next->term.value.variable);
		}

		tl_ptr=tl_ptr->next;
	}

	return vl;
}



void structure_unify(VariableTable v1,Structure s1,VariableTable v2,Structure s2){
	TermList* s1_ptr;
	TermList* s2_ptr;

	if(s1.functor!=s2.functor){
		error("unification error.");
	}

	if(structure_arity(s1)!=structure_arity(s2)){
		error("unification error.");
	}

	s1_ptr=&(s1.arguments); s2_ptr=&(s2.arguments);

	while(s1_ptr->next!=NULL){
		term_unify(v1,&(s1_ptr->next->term),v2,&(s2_ptr->next->term));

		s1_ptr=s1_ptr->next;
		s2_ptr=s2_ptr->next;
	}

	return;
}

void term_unify(VariableTable vl_caller,Term* caller,VariableTable vl_callee,Term* callee){
	if(caller->tag==TERM_INTEGER && callee->tag==TERM_INTEGER){
		return;
	}else if(caller->tag==TERM_STRUCTURE && callee->tag==TERM_STRUCTURE){
		structure_unify(vl_caller,*(caller->value.structure),vl_callee,*(callee->value.structure));
		return;
	}else if(caller->tag==TERM_VARIABLE && callee->tag==TERM_VARIABLE){
		term_unify(vl_caller,vartable_find(vl_caller,caller->value.variable),vl_callee,vartable_find(vl_callee,callee->value.variable));
		return;
	}else if(caller->tag==TERM_VARIABLE){
		term_unify(vl_caller,vartable_find(vl_caller,caller->value.variable),vl_callee,callee);
		return;
	}else if(callee->tag==TERM_VARIABLE){
		term_unify(vl_caller,caller,vl_callee,vartable_find(vl_callee,callee->value.variable));
		return;
	}else if(caller->tag==TERM_UNBOUND && callee->tag==TERM_UNBOUND){
		callee->tag=TERM_POINTER;
		callee->value.pointer=caller;
		return;
	}else if(caller->tag==TERM_UNBOUND){
		*caller=*callee;
		return;
	}else if(callee->tag==TERM_UNBOUND){
		*callee=*caller;
		return;
	}else if(caller->tag==TERM_POINTER && callee->tag==TERM_POINTER){
		term_unify(vl_caller,caller->value.pointer,vl_callee,callee->value.pointer);
		return;
	}else if(caller->tag==TERM_POINTER){
		term_unify(vl_caller,caller->value.pointer,vl_callee,callee);
		return;
	}else if(callee->tag==TERM_POINTER){
		term_unify(vl_caller,caller,vl_callee,callee->value.pointer);
		return;
	}

	error("unification error.");
}

int structure_unify_test(VariableTable v1,Structure s1,VariableTable v2,Structure s2){
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
		if(!term_unify_test(v1,s1_ptr->next->term,v2,s2_ptr->next->term)){
			return 0;
		}

		s1_ptr=s1_ptr->next;
		s2_ptr=s2_ptr->next;
	}

	return 1;
}

int term_unify_test(VariableTable vl_caller,Term caller,VariableTable vl_callee,Term callee){
	if(caller.tag==TERM_INTEGER && callee.tag==TERM_INTEGER){
		return caller.value.integer==callee.value.integer;
	}else if(caller.tag==TERM_STRUCTURE && callee.tag==TERM_STRUCTURE){
		return structure_unify_test(vl_caller,*(caller.value.structure),vl_callee,*(callee.value.structure));
	}else if(caller.tag==TERM_VARIABLE && callee.tag==TERM_VARIABLE){
		return term_unify_test(vl_caller,*vartable_find(vl_caller,caller.value.variable),vl_callee,*vartable_find(vl_callee,callee.value.variable));
	}else if(caller.tag==TERM_VARIABLE){
		return term_unify_test(vl_caller,*vartable_find(vl_caller,caller.value.variable),vl_callee,callee);
	}else if(callee.tag==TERM_VARIABLE){
		return term_unify_test(vl_caller,caller,vl_callee,*vartable_find(vl_callee,callee.value.variable));
	}else if(caller.tag==TERM_UNBOUND || callee.tag==TERM_UNBOUND){
		return 1;
	}else if(caller.tag==TERM_POINTER && callee.tag==TERM_POINTER){
		return term_unify_test(vl_caller,*(caller.value.pointer),vl_callee,*(callee.value.pointer));
	}else if(caller.tag==TERM_POINTER){
		return term_unify_test(vl_caller,*(caller.value.pointer),vl_callee,callee);
	}else if(callee.tag==TERM_POINTER){
		return term_unify_test(vl_caller,caller,vl_callee,*(callee.value.pointer));
	}

	return 0; //単一化できない（t1:数値 t2:構造 などの組み合わせ）
}

