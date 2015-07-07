#pragma once

#include <stdio.h>

#define SYMTABLE_LEN 64
#define IDENT_MAX_STR_LEN 256


typedef struct _symtable{
	struct _symtable* next;
	char* name;
} SymbolTable;

typedef SymbolTable* Atom;
typedef SymbolTable* Variable;

typedef enum{TOK_INTEGER,TOK_ATOM,TOK_VARIABLE,TOK_ASCII,TOK_QUESTION,TOK_IMPLICATION,TOK_ENDOFFILE} TokenTag;
typedef enum{TERM_ATOM,TERM_INTEGER,TERM_VARIABLE,TERM_STRUCTURE} TermTag;

typedef struct _token{
	//QUESTION ?-
	//IMPLICATION :-
	//ASCII . + * , ( ) etc...
	TokenTag tag;
	union{
		int integer;
		Atom atom;
		Variable variable;
		char ascii;
	} value;
} Token;

struct _structure;

typedef struct _structure_list{
	struct _structure_list* next;
	struct _structure* structure;
} StructureList;

typedef struct _question{
	StructureList structlist;
} Question;

typedef struct _clause{
	struct _structure* head;
	StructureList body;
} Clause;

typedef struct _term{
	TermTag tag;
	union {
		Atom atom;
		int integer;
		Variable variable;
		struct _structure* structure;
	} value;
} Term;

typedef struct _term_list{
	struct _term_list* next;
	Term term;
} TermList;

typedef struct _structure{
	Atom functor;
	TermList termlist;
} Structure;

typedef struct _program{
	struct _program* next;
	enum{CLAUSE,QUESTION} tag;
	union{
		Clause clause;
		Question question;
	} item;
} Program;


//etc.c
void error(char* msg);

//symbol.c
extern SymbolTable symtable[SYMTABLE_LEN];
SymbolTable* sym_get(char* str);

//lexer.c
Token token_get(FILE* in);


