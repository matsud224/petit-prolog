#include <stdio.h>
#include <malloc.h>
#include "header.h"


Program parse_program(FILE* fp){
    Program program_root; program_root.next=NULL;
    Program* program_terminal=&program_root; //連結リストの末端
    Program* newprog;
    Token gottoken;

    while(gottoken=token_get(fp),gottoken.tag !=TOK_ENDOFFILE){
        if(gottoken.tag==TOK_QUESTION){
            //?-
            newprog=malloc(sizeof(Program));
            newprog->tag=PROG_QUESTION;
            newprog->item.question.body=parse_structure_list(fp);
            program_terminal->next=newprog;
            program_terminal=newprog;
            program_terminal->next=NULL;
        }else{
            //:-
            token_unget(gottoken);

            newprog=malloc(sizeof(Program));
            newprog->tag=PROG_CLAUSE;
            newprog->item.clause.head=parse_structure(fp);

            gottoken=token_get(fp);
            if(gottoken.tag==TOK_IMPLICATION){
                // :-付きの場合
                newprog->item.clause.body=parse_structure_list(fp);
            }else{
                newprog->item.clause.body.next=NULL;
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

    return program_root;
}

StructureList parse_structure_list(FILE* fp){
    StructureList st_root; st_root.next=NULL;
    StructureList* st_terminal=&st_root;
    Token gottoken;
    while(1){
        st_terminal->next=malloc(sizeof(StructureList));
        st_terminal->next->structure=parse_structure(fp);
        st_terminal=st_terminal->next;
        st_terminal->next=NULL;
        gottoken=token_get(fp);
        if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!=','){
            break;
        }
    }

    token_unget(gottoken);

    return st_root;
}

Structure parse_structure(FILE* fp){
    Token gottoken=token_get(fp);
    Structure newstruct;

    //printf("parse structure\n");

    if(gottoken.tag==TOK_ASCII && gottoken.value.ascii=='['){
		//printf("in structure, list!");
		token_unget(gottoken);
		return parse_list(fp);
    }else if(gottoken.tag!=TOK_ATOM){
        error("atom expected before lparen.");
    }

    newstruct.functor=gottoken.value.atom;

    //開きカッコの確認
    gottoken=token_get(fp);
    if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!='('){
        //arity 0 (かっこ付けない)
        token_unget(gottoken);
        newstruct.arguments.next=NULL;
        return newstruct;
    }

    newstruct.arguments=parse_term_list(fp);

    //閉じカッコの確認
    gottoken=token_get(fp);
    if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!=')'){
        error("rparen expected.");
    }

    return newstruct;
}

Structure parse_list_sub(FILE* fp){
    Token gottoken;
    Structure newstruct;

    //printf("sub\n");

    newstruct.functor=sym_get(".");

    newstruct.arguments.next=malloc(sizeof(TermList));
    newstruct.arguments.next->term=parse_term(fp);
    newstruct.arguments.next->next=malloc(sizeof(TermList));
    newstruct.arguments.next->next->next=NULL;

    gottoken=token_get(fp);
    if(gottoken.tag==TOK_ASCII && gottoken.value.ascii==','){
		//printf("comma\n");
		newstruct.arguments.next->next->term.tag=TERM_STRUCTURE;
		newstruct.arguments.next->next->term.value.structure=malloc(sizeof(Structure));
		*(newstruct.arguments.next->next->term.value.structure)=parse_list_sub(fp);
    }else if(gottoken.tag==TOK_ASCII && gottoken.value.ascii=='|'){
		//printf("ascii-> |\n");
		newstruct.arguments.next->next->term=parse_term(fp);
    }else{
    	//printf("end of list.\n");
    	token_unget(gottoken);
		newstruct.arguments.next->next->term.tag=TERM_STRUCTURE;
		newstruct.arguments.next->next->term.value.structure=malloc(sizeof(Structure));
		newstruct.arguments.next->next->term.value.structure->functor=sym_get("[]");
		newstruct.arguments.next->next->term.value.structure->arguments.next=NULL;
    }

    return newstruct;
}

Structure parse_list(FILE* fp){
    Token gottoken=token_get(fp);
    Structure newstruct;

    //開きカッコの確認
    if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!='['){
        error("lbracket expected.");
    }

    gottoken=token_get(fp);

    if(gottoken.tag==TOK_ASCII && gottoken.value.ascii==']'){
		// [] はアトムとして特別扱い
		newstruct.functor=sym_get("[]");
		newstruct.arguments.next=NULL;
		return newstruct;
    }

	token_unget(gottoken);

	newstruct=parse_list_sub(fp);

    //閉じカッコの確認
    gottoken=token_get(fp);
    if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!=']'){
        error("rbracket expected.");
    }

    return newstruct;
}



Term parse_term(FILE* fp){
    Term t;
    Token gottoken=token_get(fp);

	//printf("parse term : %d,%c\n",gottoken.tag,gottoken.value.ascii);

    if(gottoken.tag==TOK_INTEGER){
        t.tag=TERM_INTEGER; t.value.integer=gottoken.value.integer;
    }else if(gottoken.tag==TOK_VARIABLE){
        t.tag=TERM_VARIABLE; t.value.variable=gottoken.value.variable;
    }else if(gottoken.tag==TOK_ATOM){
        token_unget(gottoken);
        t.tag=TERM_STRUCTURE;
        t.value.structure=malloc(sizeof(Structure));
        *(t.value.structure)=parse_structure(fp);
    }else if(gottoken.tag==TOK_ASCII && gottoken.value.ascii=='['){
    	//printf("list\n");
		token_unget(gottoken);
        t.tag=TERM_STRUCTURE;
        t.value.structure=malloc(sizeof(Structure));
        *(t.value.structure)=parse_structure(fp);
    }else{
    	//printf("ascii:%c",gottoken.value.ascii);
        error("term expected.");
    }

    return t;
}

TermList parse_term_list(FILE* fp){
    TermList tl_root;tl_root.next=NULL;
    TermList* tl_terminal=&tl_root;
    Token gottoken;
    //printf("parse term list\n");
    while(1){
        tl_terminal->next=malloc(sizeof(TermList));
        tl_terminal->next->term=parse_term(fp);
        tl_terminal=tl_terminal->next;
        tl_terminal->next=NULL;

        gottoken=token_get(fp);
        if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!=','){
            break;
        }
    }

    token_unget(gottoken);

    return tl_root;
}
