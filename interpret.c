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
	//clauseの引数の数を数える
	int arity=0;
	TermList* t_ptr=&clause.head.arguments;
	while(t_ptr->next!=NULL){
		arity++;
		t_ptr=t_ptr->next;
	}
	clause.arity=arity;

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

void interpret_question(Question question){

}
