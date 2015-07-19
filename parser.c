#include <stdio.h>
#include <malloc.h>
#include "header.h"


Program* parse_program(FILE* fp){
	gcmemstack_pushnew(&GCMEMSTACK);

    Program* program_root=gc_malloc(sizeof(Program),F_PROGRAM,&GCMEMSTACK); program_root->next=NULL;
    Program* program_terminal=program_root; //連結リストの末端
    Program* newprog;
    Token gottoken;

    while(gottoken=token_get(fp),gottoken.tag !=TOK_ENDOFFILE){
        if(gottoken.tag==TOK_QUESTION){
            //?-
            newprog=gc_malloc(sizeof(Program),F_PROGRAM,&GCMEMSTACK);
            newprog->tag=PROG_QUESTION;
            newprog->item.question=gc_malloc(sizeof(Question),F_QUESTION,&GCMEMSTACK);
            newprog->item.question->body=parse_structure_list(fp);
            program_terminal->next=newprog;
            program_terminal=newprog;
            program_terminal->next=NULL;
        }else{
            //:-
            token_unget(gottoken);

            newprog=gc_malloc(sizeof(Program),F_PROGRAM,&GCMEMSTACK);
            newprog->tag=PROG_CLAUSE;
            newprog->item.clause=gc_malloc(sizeof(Clause),F_CLAUSE,&GCMEMSTACK);
            newprog->item.clause->head=parse_structure(fp);

            gottoken=token_get(fp);
            if(gottoken.tag==TOK_IMPLICATION){
                // :-付きの場合
                newprog->item.clause->body=parse_structure_list(fp);
            }else{
            	newprog->item.clause->body=gc_malloc(sizeof(StructureList),F_STRUCTURELIST,&GCMEMSTACK);
                newprog->item.clause->body->next=NULL;
                token_unget(gottoken);
            }

            program_terminal->next=newprog;
            program_terminal=newprog;
            program_terminal->next=NULL;
        }

        gottoken=token_get(fp);
        if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!='.'){
            error("dot expected.");
        }
    }

    gcmemstack_returnptr(program_root,&GCMEMSTACK);

    gcmemstack_pop(&GCMEMSTACK);

    return program_root;
}

StructureList* parse_structure_list(FILE* fp){
	gcmemstack_pushnew(&GCMEMSTACK);
    StructureList* st_root=gc_malloc(sizeof(StructureList),F_STRUCTURELIST,&GCMEMSTACK); st_root->next=NULL;
    StructureList* st_terminal=st_root;
    Token gottoken;
    while(1){
        st_terminal->next=gc_malloc(sizeof(StructureList),F_STRUCTURELIST,&GCMEMSTACK);
        st_terminal->next->structure=parse_structure(fp);
        st_terminal->next->next=NULL;
        st_terminal=st_terminal->next;
        st_terminal->next=NULL;
        gottoken=token_get(fp);
        if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!=','){
            break;
        }
    }

    token_unget(gottoken);

    gcmemstack_returnptr(st_root,&GCMEMSTACK);

	gcmemstack_pop(&GCMEMSTACK);
    return st_root;
}

Structure* parse_structure(FILE* fp){
	gcmemstack_pushnew(&GCMEMSTACK);
    Token gottoken=token_get(fp);
    Structure* newstruct=gc_malloc(sizeof(Structure),F_STRUCTURE,&GCMEMSTACK);
	newstruct->arguments=gc_malloc(sizeof(TermList),F_TERMLIST,&GCMEMSTACK);
	newstruct->arguments->next=NULL;

    if(gottoken.tag==TOK_ASCII && gottoken.value.ascii=='['){
		token_unget(gottoken);
		gcmemstack_pop(&GCMEMSTACK);
		return parse_list(fp);
    }else if(gottoken.tag!=TOK_ATOM){
        error("atom expected before lparen.");
    }

    newstruct->functor=gottoken.value.atom;

    //開きカッコの確認
    gottoken=token_get(fp);
    if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!='('){
        //arity 0 (かっこ付けない)
        token_unget(gottoken);
        newstruct->arguments->next=NULL;
        gcmemstack_returnptr(newstruct,&GCMEMSTACK);
        gcmemstack_pop(&GCMEMSTACK);
        return newstruct;
    }

    newstruct->arguments=parse_term_list(fp);

    //閉じカッコの確認
    gottoken=token_get(fp);
    if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!=')'){
        error("rparen expected.");
    }

	gcmemstack_returnptr(newstruct,&GCMEMSTACK);
	gcmemstack_pop(&GCMEMSTACK);
    return newstruct;
}

Structure* parse_list_sub(FILE* fp){
	gcmemstack_pushnew(&GCMEMSTACK);

    Token gottoken;
    Structure* newstruct=gc_malloc(sizeof(Structure),F_STRUCTURE,&GCMEMSTACK);

    newstruct->functor=sym_get(".");

	newstruct->arguments=gc_malloc(sizeof(TermList),F_TERMLIST,&GCMEMSTACK);
    newstruct->arguments->next=gc_malloc(sizeof(TermList),F_TERMLIST,&GCMEMSTACK);
    newstruct->arguments->next->term=parse_term(fp);
    newstruct->arguments->next->next=gc_malloc(sizeof(TermList),F_TERMLIST,&GCMEMSTACK);
    newstruct->arguments->next->next->next=NULL;

    gottoken=token_get(fp);
    if(gottoken.tag==TOK_ASCII && gottoken.value.ascii==','){
		newstruct->arguments->next->next=gc_malloc(sizeof(TermList),F_TERMLIST,&GCMEMSTACK);
		newstruct->arguments->next->next->next=NULL;
    	newstruct->arguments->next->next->term=gc_malloc(sizeof(Term),F_TERM,&GCMEMSTACK);
		newstruct->arguments->next->next->term->tag=TERM_STRUCTURE;
		newstruct->arguments->next->next->term->value.structure=parse_list_sub(fp);
    }else if(gottoken.tag==TOK_ASCII && gottoken.value.ascii=='|'){
    	newstruct->arguments->next->next=gc_malloc(sizeof(TermList),F_TERMLIST,&GCMEMSTACK);
		newstruct->arguments->next->next->term=parse_term(fp);
    }else{
    	token_unget(gottoken);
    	newstruct->arguments->next->next=gc_malloc(sizeof(TermList),F_TERMLIST,&GCMEMSTACK);
		newstruct->arguments->next->next->next=NULL;
    	newstruct->arguments->next->next->term=gc_malloc(sizeof(Term),F_TERM,&GCMEMSTACK);
		newstruct->arguments->next->next->term->tag=TERM_STRUCTURE;
		newstruct->arguments->next->next->term->value.structure=gc_malloc(sizeof(Structure),F_STRUCTURE,&GCMEMSTACK);
		newstruct->arguments->next->next->term->value.structure->functor=sym_get("[]");
		newstruct->arguments->next->next->term->value.structure->arguments=gc_malloc(sizeof(TermList),F_TERMLIST,&GCMEMSTACK);
		newstruct->arguments->next->next->term->value.structure->arguments->next=NULL;
    }

	gcmemstack_returnptr(newstruct,&GCMEMSTACK);
	gcmemstack_pop(&GCMEMSTACK);
    return newstruct;
}

Structure* parse_list(FILE* fp){
	gcmemstack_pushnew(&GCMEMSTACK);

    Token gottoken=token_get(fp);
    Structure* newstruct=gc_malloc(sizeof(Structure),F_STRUCTURE,&GCMEMSTACK);
    newstruct->arguments=gc_malloc(sizeof(TermList),F_TERMLIST,&GCMEMSTACK);
    newstruct->arguments->next=NULL;

    //開きカッコの確認
    if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!='['){
        error("lbracket expected.");
    }

    gottoken=token_get(fp);

    if(gottoken.tag==TOK_ASCII && gottoken.value.ascii==']'){
		// [] はアトムとして特別扱い
		newstruct->functor=sym_get("[]");
		newstruct->arguments->next=NULL;

		gcmemstack_returnptr(newstruct,&GCMEMSTACK);
		gcmemstack_pop(&GCMEMSTACK);
		return newstruct;
    }

	token_unget(gottoken);

	newstruct=parse_list_sub(fp);

    //閉じカッコの確認
    gottoken=token_get(fp);
    if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!=']'){
        error("rbracket expected.");
    }

	gcmemstack_returnptr(newstruct,&GCMEMSTACK);
    gcmemstack_pop(&GCMEMSTACK);

    return newstruct;
}



Term* parse_term(FILE* fp){
	gcmemstack_pushnew(&GCMEMSTACK);

    Term* t=gc_malloc(sizeof(Term),F_TERM,&GCMEMSTACK);
    Token gottoken=token_get(fp);

    if(gottoken.tag==TOK_INTEGER){
        t->tag=TERM_INTEGER; t->value.integer=gottoken.value.integer;
    }else if(gottoken.tag==TOK_VARIABLE){
        t->tag=TERM_VARIABLE; t->value.variable=gottoken.value.variable;
    }else if(gottoken.tag==TOK_ATOM){
        token_unget(gottoken);
        t->tag=TERM_STRUCTURE;
        t->value.structure=parse_structure(fp);
    }else if(gottoken.tag==TOK_ASCII && gottoken.value.ascii=='['){
		token_unget(gottoken);
        t->tag=TERM_STRUCTURE;
        t->value.structure=parse_structure(fp);
    }else{
        error("term expected.");
    }

	gcmemstack_returnptr(t,&GCMEMSTACK);
    gcmemstack_pop(&GCMEMSTACK);

    return t;
}

TermList* parse_term_list(FILE* fp){
	gcmemstack_pushnew(&GCMEMSTACK);
    TermList* tl_root=gc_malloc(sizeof(TermList),F_TERMLIST,&GCMEMSTACK); tl_root->next=NULL;
    TermList* tl_terminal=tl_root;
    Token gottoken;

    while(1){
        tl_terminal->next=gc_malloc(sizeof(TermList),F_TERMLIST,&GCMEMSTACK);
        tl_terminal->next->next=NULL;
        tl_terminal->next->term=parse_term(fp);
        tl_terminal=tl_terminal->next;
        tl_terminal->next=NULL;

        gottoken=token_get(fp);
        if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!=','){
            break;
        }
    }

    token_unget(gottoken);

	gcmemstack_returnptr(tl_root,&GCMEMSTACK);
	gcmemstack_pop(&GCMEMSTACK);

    return tl_root;
}
