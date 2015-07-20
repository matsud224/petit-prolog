#pragma once


#define SYMTABLE_LEN 128
#define IDENT_MAX_STR_LEN 256


struct _clause;
struct _vartable;

typedef struct _gcmemlist{
    struct _gcmemlist* next;
    void* memptr;
} GCMemoryList;

typedef struct _gcmemstack{
    struct _gcmemstack* next;
    GCMemoryList memlist;
} GCMemoryStack;

typedef struct _clauselist{
    struct _clauselist* next;
    struct _clause* clause;
} ClauseList;

typedef struct _symtable{
	struct _symtable* next;
	char* name;
	ClauseList* clause_list;
} SymbolTable;

typedef struct _gcheader{
	unsigned char fieldtag:4;
	unsigned char marked:1;
	size_t fieldsize;
	struct _gcheader* next;
} GCHeader;

typedef struct _chanklist{
	struct _chanklist* next;
	char* chank;
} ChankList;

typedef SymbolTable* Atom;
typedef SymbolTable* Variable;

typedef enum{ASSOC_LEFT,ASSOC_RIGHT} Assoc;
typedef enum{OP_UNARY,OP_BINARY} OperatorType;

typedef struct _operator{
	Atom atom;
	int prec;
	Assoc assoc;
	OperatorType type;
} Operator;

typedef struct _operatorlist{
	struct _operatorlist* next;
	Operator* operator;
} OperatorList;

typedef enum{TOK_INTEGER,TOK_ATOM,TOK_VARIABLE,TOK_ASCII,TOK_QUESTION,TOK_IMPLICATION,TOK_ENDOFFILE} TokenTag;
typedef enum{TERM_INTEGER,TERM_VARIABLE,TERM_STRUCTURE,TERM_PPTERM,TERM_UNBOUND} TermTag;
typedef enum{PROG_CLAUSE,PROG_QUESTION} ProgramTag;
typedef enum{F_NULL,F_HISTORYTABLE,F_VARIABLETABLE,F_TERM,F_HTSTACK,F_CLAUSE,F_CLAUSELIST,F_BOX
				,F_STRUCTURE,F_TERMLIST,F_PROGRAM,F_STRUCTURELIST,F_SYMBOLTABLE,F_CHARARRAY,F_QUESTION} FieldTag;

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
		struct _vartable* ppterm;
	} value;
} Term;

typedef struct _term_list{
	struct _term_list* next;
	Term* term;
} TermList;

typedef struct _structure{
	Atom functor;
	TermList* arguments;
} Structure;

typedef struct _structure_list{
	struct _structure_list* next;
	struct _structure* structure;
} StructureList;

typedef struct _question{
	StructureList* body;
} Question;

typedef struct _clause{
	int arity; //登録段階で代入
	struct _structure* head;
	StructureList* body;
} Clause;

typedef struct _program{
	struct _program* next;
	ProgramTag tag;
	union{
		Clause *clause;
		Question *question;
	} item;
} Program;

typedef struct _vartable{
	struct _vartable* next;
	Variable variable;
	Term* termptr;
} VariableTable;

typedef struct _history{
	struct _history* next;
	struct _vartable* ppterm;
	Term* pterm;
	Term* prev;
} HistoryTable;

typedef struct _htstack{
	struct _htstack* next;
	HistoryTable* htable;
} HTStack;

typedef struct _box{
	struct _box* success;
	struct _box* failure;
	struct _box* brother;
	ClauseList* selected_clause;
	VariableTable* vartable;
	Structure* structure;
	char is_begin:1;
	char is_end:1;
	char is_failed:1;
} Box;


//etc.c
void error(char* msg);
void vartable_show(VariableTable v1);
void term_show(Term* t);
void structure_show(Structure s);

void htable_add(HistoryTable *vl,Term* pterm);
void htable_addforward(HistoryTable *vl,VariableTable* ppterm,Term* prev);

void vartable_addvar(VariableTable *vl,Variable var);
VariableTable* vartable_findvar(VariableTable* vl,Variable var);
int vartable_hasitem(VariableTable* v1);

void htstack_pushnew(HTStack *hts);
void htstack_push(HTStack *hts,HistoryTable* htable);
void htstack_pop(HTStack *hts);
HistoryTable* htstack_toptable(HTStack hts);

//symbol.c
extern SymbolTable symtable[SYMTABLE_LEN];
SymbolTable* sym_get(char* str);
SymbolTable* sym_get_anonymousvar();

//lexer.c
Token token_get(FILE* fp);
void token_unget(Token t);

//parser.c
Program* parse_program(FILE* fp);
StructureList* parse_structure_list(FILE* fp);
Structure* parse_structure(FILE* fp);
Structure* parse_list(FILE* fp);
Term* parse_term(FILE* fp);
TermList* parse_term_list(FILE* fp);

//interpret.c
extern Box* CURRENT_BEGINBOX;
extern HTStack GlobalStack;
int structure_arity(Structure* s);
void interpret(FILE* fp);
void interpret_clause(Clause* clause);
void interpret_question(Question* question);
void execute(Box* current);
VariableTable* next_clause(Box* box);
void vartable_from_clause(VariableTable* vt,Clause* c);
void vartable_from_question(VariableTable* vt,Question* q);
void vartable_from_structure(VariableTable* vt,Structure* s);
Term* term_to_portable(Term* t,VariableTable* vt);
Structure* structure_to_portable(Structure* s,VariableTable* vt);
Term* term_unwrap(Term* t,VariableTable vt);
int structure_unify(Structure* s1,Structure* s2,VariableTable* v1,VariableTable* v2,HistoryTable* h);
int term_unify(Term* t1,Term* t2,VariableTable* v1,VariableTable* v2,HistoryTable* h);
Term* term_remove_ppterm(Term* t);

//gc.c
extern int SHOW_GCINFO;
extern GCMemoryStack GCMEMSTACK;
void gc_init();
void gc_mark_sub(void* m);
void gc_mark();
void gc_sweep();
void* gc_malloc(size_t size,FieldTag tag,GCMemoryStack* gcms);
void gc_freelist_show();
size_t gc_freesize();
void gc_chankallocate();
void gcmemlist_add(GCMemoryList* ml,void* mem);
GCMemoryList* gcmemstack_top(GCMemoryStack* gcms);
void gcmemstack_pushnew(GCMemoryStack *gcms);
void gcmemstack_pop(GCMemoryStack *gcms);
void gcmemstack_returnptr(void* mem,GCMemoryStack *gcms);
int gcmemstack_size(GCMemoryStack *gcms);
int gcmemlist_size(GCMemoryList *gcml);
