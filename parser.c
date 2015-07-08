#include <stdio.h>
#include <malloc.h>
#include "header.h"


Program parse_program(FILE* in){
    Program program_root; program_root.next=NULL;
    Program* program_terminal=&program_root; //連結リストの末端
    Program* newprog;
    Token gottoken;

    while(gottoken=token_get(in),gottoken.tag !=TOK_ENDOFFILE){
        printf("hahahahaha\n");
        if(gottoken.tag==TOK_QUESTION){
            //?-
            newprog=malloc(sizeof(Program));
            newprog->tag=PROG_QUESTION;
            newprog->item.question.body=parse_structure_list(in);
            program_terminal->next=newprog;
            program_terminal=newprog;
            program_terminal->next=NULL;
        }else{
            //:-
            token_unget(gottoken);

            newprog=malloc(sizeof(Program));
            newprog->tag=PROG_CLAUSE;
            newprog->item.clause.head=parse_structure(in);

            gottoken=token_get(in);
            if(gottoken.tag==TOK_IMPLICATION){
                // :-付きの場合
                printf("impl\n");
                newprog->item.clause.body=parse_structure_list(in);
            }else{
                printf("no-impl\n");
                newprog->item.clause.body.next=NULL;
                token_unget(gottoken);
            }

            program_terminal->next=newprog;
            program_terminal=newprog;
            program_terminal->next=NULL;
        }

        gottoken=token_get(in);
        if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!='.'){
            error("dot expected.");
        }
    }

    return program_root;
}

StructureList parse_structure_list(FILE* in){
    StructureList st_root; st_root.next=NULL;
    StructureList* st_terminal=&st_root;
    Token gottoken;
    while(1){
        st_terminal->next=malloc(sizeof(StructureList));
        st_terminal->next->structure=parse_structure(in);
        st_terminal=st_terminal->next;
        st_terminal->next=NULL;
        gottoken=token_get(in);
        if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!=','){
            break;
        }
    }

    token_unget(gottoken);

    return st_root;
}

Structure parse_structure(FILE* in){
    Token gottoken=token_get(in);
    Structure newstruct;
    if(gottoken.tag!=TOK_ATOM){
        error("atom expected before lparen.");
    }

    newstruct.functor=gottoken.value.atom;

    //開きカッコの確認
    gottoken=token_get(in);
    if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!='('){
        //arity 0 (かっこ付けない)
        token_unget(gottoken);
        newstruct.arguments.next=NULL;
        return newstruct;
    }

    newstruct.arguments=parse_term_list(in);

    //閉じカッコの確認
    gottoken=token_get(in);
    if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!=')'){
        error("rparen expected.");
    }

    return newstruct;
}

Term parse_term(FILE* in){
    Term t;
    Token gottoken=token_get(in);

    if(gottoken.tag==TOK_INTEGER){
        t.tag=TERM_INTEGER; t.value.integer=gottoken.value.integer;
    }else if(gottoken.tag==TOK_VARIABLE){
        t.tag=TERM_VARIABLE; t.value.variable=gottoken.value.variable;
    }else if(gottoken.tag==TOK_ATOM){
        token_unget(gottoken);
        t.tag=TERM_STRUCTURE;
        t.value.structure=malloc(sizeof(Structure));
        *(t.value.structure)=parse_structure(in);
    }else{
        error("term expected.");
    }

    return t;
}

TermList parse_term_list(FILE* in){
    TermList tl_root;tl_root.next=NULL;
    TermList* tl_terminal=&tl_root;
    Token gottoken;
    while(1){
        tl_terminal->next=malloc(sizeof(TermList));
        tl_terminal->next->term=parse_term(in);
        tl_terminal=tl_terminal->next;
        tl_terminal->next=NULL;
        gottoken=token_get(in);
        if(gottoken.tag!=TOK_ASCII || gottoken.value.ascii!=','){
            break;
        }
    }

    token_unget(gottoken);

    return tl_root;
}
