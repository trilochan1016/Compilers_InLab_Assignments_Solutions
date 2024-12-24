%{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int yylex ( );

/* Symbol table */
typedef struct {
   int type;       /* NUM or ID */
   char *data;     /* Numeric string (if type = NUM) or identifier name (if type = ID) */
   int value;      /* Numeric value of data */
   int init;       /* Initialized (1) or not (0) for type = ID, 1 for NUM */
} stentry;
stentry ST[1024];  /* Symbol table capable of storing at most 1024 NUM and ID */
int nsymb = 0;     /* Number of entries in the symbol table */

int addsymbol ( int , char * ) ;  /* Add an entry to symbol table *if not already present)
                                     Return the index in the symbol table */
int setsymbol ( int , int ) ;     /* Set numeric value to an ID-type entry in symbol table */
int getsymbol ( char * ) ;        /* Get numeric value of symbol */
int getvalue ( int ) ;            /* Get numeric value stored in ST[i] */

int getvalue ( int ) ;            /* Get numeric value stored in ST[i] */
/* Expression tree */
typedef struct _node {
   int type;       /* OP for non-leaf nodes, ID or NUM for leaf nodes */
   int data;       /* Opcode for type = OP, index in the symbol table for type = ID or NUM */
   struct _node *L, *R;
} node;
node *ET;          /* Global expression tree */

node *addnode ( int , node * , node * ) ;  /* Create new non-leaf node
                                              Patameters: opcode, left child, right child */
node *addleaf ( int ) ;     /* Add leaf for an entry in symbol table */
int evaltree ( node * ) ;   /* Numeric evaluation of an expression tree */
%}

%start program
%union { int opcode; int punc; int value; int idx; int kwd; char *text; struct _node *exprtree; }
%token <text> NUM
%token <text> ID
%token <opcode> ADD
%token <opcode> SUB
%token <opcode> MUL
%token <opcode> DIV
%token <opcode> REM
%token <opcode> POW
%token <opcode> OP
%token <punc> LP
%token <punc> RP
%token <kwd> SET
%type <exprtree> expr arg
%type <opcode> op
%type <value> exprstmt
%type <idx> setstmt

%%

program	: stmt program
	| stmt
	;

stmt	: setstmt		{ printf("Variable %s is set to %d\n", ST[$1].data, ST[$1].value); }
	| exprstmt		{ printf("Standalone expression evaluates to %d\n", $1); }
	;

setstmt	: LP SET ID NUM RP	{ $$ = setsymbol(addsymbol(ID,$3),atoi($4)); }
	| LP SET ID ID RP	{ $$ = setsymbol(addsymbol(ID,$3),getsymbol($4)); }
	| LP SET ID expr RP	{ $$ = setsymbol(addsymbol(ID,$3),evaltree($4)); ET = NULL; }
	;

exprstmt : expr			{ $$ = evaltree($1); ET = NULL; }
	;

expr	: LP op arg arg RP	{ $$ = addnode($2,$3,$4); }
	;

op	: ADD			{ $$ = $1; }
	| SUB			{ $$ = $1; }
	| MUL			{ $$ = $1; }
	| DIV			{ $$ = $1; }
	| REM			{ $$ = $1; }
	| POW			{ $$ = $1; }
	;

arg	: NUM			{ $$ = addleaf(addsymbol(NUM,$1)); }
	| ID			{ $$ = addleaf(addsymbol(ID,$1)); }
	| expr			{ $$ = $1; }
	;

%%

void yyerror ( char *msg ) { fprintf(stderr, "Error: %s\n", msg); }

