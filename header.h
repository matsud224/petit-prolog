#pragma once

#include <stdio.h>

#define SYMTABLE_LEN 64
#define IDENT_MAX_STR_LEN 256


struct _clause;

typedef struct _clauselist{
    struct _clauselist* next;
    struct _clause* clause;
} ClauseList;

typedef struct _symtable{
	struct _symtable* next;
	char* name;
	ClauseList clause_list;
} SymbolTable;

typedef SymbolTable* Atom;
typedef SymbolTable* Variable;

typedef enum{TOK_INTEGER,TOK_ATOM,TOK_VARIABLE,TOK_ASCII,TOK_QUESTION,TOK_IMPLICATION,TOK_ENDOFFILE} TokenTag;
typedef enum{TERM_INTEGER,TERM_VARIABLE,TERM_STRUCTURE,TERM_POINTER,TERM_UNBOUND} TermTag;
typedef enum{PROG_CLAUSE,PROG_QUESTION} ProgramTag;

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


typedef struct _term{
	TermTag tag;
	union {
		int integer;
		Variable variable;
		struct _structure* structure;
		struct _term* pointer;
	} value;
} Term;

typedef struct _term_list{
	struct _term_list* next;
	Term term;
} TermList;

typedef struct _structure{
	Atom functor;
	TermList arguments;
} Structure;

typedef struct _structure_list{
	struct _structure_list* next;
	struct _structure structure;
} StructureList;

typedef struct _question{
	StructureList body;
} Question;

typedef struct _clause{
	int arity; //登録段階で代入
	struct _structure head;
	StructureList body;
} Clause;

typedef struct _program{
	struct _program* next;
	ProgramTag tag;
	union{
		Clause clause;
		Question question;
	} item;
} Program;

typedef struct _vartable{
	struct _vartable* next;
	Variable variable;
	Term value;
} VariableTable;

typedef struct _vtstack{
	struct _vtstack* next;
	VariableTable vartable;
} VTStack;

typedef struct _box{
	struct _box* success;
	struct _box* failure;
	VTStack vt_stack;
	ClauseList* selected_clause;
	Structure structure;
	char is_begin:1;
	char is_end:1;
	char is_failed:1;
} Box;


//etc.c
void error(char* msg);
void vartable_show(VariableTable v1);
void vartable_add(VariableTable *vl,Variable var);
VariableTable vartable_copy(VariableTable vl);
void vartable_concat(VariableTable *dest,VariableTable src);
void vartable_unique(VariableTable vl);
Term* vartable_find(VariableTable vl,Variable var);
void vtstack_pushnew(VTStack *vts);
void vtstack_push(VTStack *vts,VariableTable vartable);
void vtstack_duplicate(VTStack *vts);
void vtstack_pop(VTStack *vts);
int vtstack_size(VTStack vts);
VariableTable* vtstack_toptable(VTStack vts);
int structure_arity(Structure s);

//symbol.c
extern SymbolTable symtable[SYMTABLE_LEN];
SymbolTable* sym_get(char* str);

//lexer.c
Token token_get(FILE* fp);
void token_unget(Token t);

//parser.c
Program parse_program(FILE* fp);
StructureList parse_structure_list(FILE* fp);
Structure parse_structure(FILE* fp);
Term parse_term(FILE* fp);
TermList parse_term_list(FILE* fp);

//interpret.c
int structure_arity(Structure s);
void interpret(FILE* fp);
void interpret_clause(Clause clause);
void interpret_question(Question question);
void execute(Box* current);
void next_clause(Box* box,VariableTable* vt_caller_ret,VariableTable* vt_callee_ret);
VariableTable vartable_from_clause(Clause c);
VariableTable vartable_from_question(Question q);
VariableTable vartable_from_structure(Structure s);

//ユニフィケーションが成功したら1,失敗なら0を返す。副作用を伴うため、ユニフィケーションできるかどうか調べるだけなら、vartable_copyでコピーを取るべし。
int structure_unify(VariableTable v1,Structure s1,VariableTable v2,Structure s2);
int term_unify(VariableTable vl_caller,Term* caller,VariableTable vl_callee,Term* callee);
