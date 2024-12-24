%{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int yylex ( );

/* Symbol table */
typedef struct {
   char *name;
   int offset;
} stentry;
stentry ST[8192];
int nsymb = 0;

int RS[12];

int TEMPNO = 0;

struct addr {
   char loc;
   int ref;
};

int OFFSET = 0;

FILE *fp = NULL;

void freeregs ( ) ;
void prnexpr ( struct addr * ) ;
struct addr *NUMtoADDR ( int ) ;
struct addr *IDtoADDR ( char * ) ;
int storeID ( char * ) ;
struct addr *fetchmem ( struct addr * ) ;
void setID ( struct addr *, struct addr * ) ;
struct addr *makeop ( int op, struct addr *, struct addr * ) ;

%}

%start program

%union { struct addr *ADDR; int OPCODE; int PUNC; int KWD; char *TEXT; int VAL; }

%token <VAL> NUM
%token <TEXT> ID
%token <OPCODE> ADD
%token <OPCODE> SUB
%token <OPCODE> MUL
%token <OPCODE> DIV
%token <OPCODE> REM
%token <OPCODE> PWR
%token <OPCODE> OP
%token <PUNC> LP
%token <PUNC> RP
%token <KWD> SET

%type <ADDR> expr arg
%type <OPCODE> op
%type <ADDR> exprstmt
%type <ADDR> setstmt

%%

program	: stmt program
	| stmt
	;

stmt	: setstmt
	| exprstmt
	;

setstmt	: LP SET ID NUM RP	{ setID(IDtoADDR($3),NUMtoADDR($4)); }
	| LP SET ID ID RP	{ setID(IDtoADDR($3),IDtoADDR($4)); }
	| LP SET ID expr RP	{ setID(IDtoADDR($3),$4); }
	;

exprstmt : expr			{ prnexpr($1); }
	;

expr	: LP op arg arg RP	{ $$ = makeop($2,$3,$4); }
	;

op	: ADD			{ $$ = $1; }
	| SUB			{ $$ = $1; }
	| MUL			{ $$ = $1; }
	| DIV			{ $$ = $1; }
	| REM			{ $$ = $1; }
	| PWR			{ $$ = $1; }
	;

arg	: NUM			{ $$ = NUMtoADDR($1); }
	| ID			{ $$ = IDtoADDR($1); }
	| expr			{ $$ = $1; }
	;

%%

void yyerror ( char *msg ) { fprintf(stderr, "Error: %s\n", msg); }
