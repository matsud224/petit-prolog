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

//Boxのメモリを確保し、それへのポインタを返します
Box* box_get(){
	Box* b=malloc(sizeof(Box));
	b->failure=NULL;
	b->success=NULL;
	b->is_begin=0;
	b->is_end=0;
	b->selected_clause=NULL;
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

}
