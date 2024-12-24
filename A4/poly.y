%{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex();

typedef struct _node {
   char symb;
   int nc;
   struct _node **C;
   int inh;
   int val;
} node;

node *PT;

node *addleaf ( char ) ;
node *addnodeN ( node *, node * ) ;
node *addnodeM ( node *, node * ) ;
node *addnodeS ( char, node * ) ;
node *addnodeP ( char, node *, node * ) ;
node *addnodeT ( char, node *, node * ) ;
node *addnodeX ( node *, node * ) ;

%}

%start S
%union { struct _node *nodep; char symb; }
%token <symb> DIG
%token <symb> ONE
%token <symb> ZERO
%token <symb> EX
%token <symb> PWR
%token <symb> PLUS
%token <symb> MINUS
%token <symb> EOL
%type <nodep> S P T X N M

%%

N	: DIG			{ $$ = addnodeN(addleaf($1),NULL); }
	| ONE M			{ $$ = addnodeN(addleaf('1'),$2); }
	| DIG M			{ $$ = addnodeN(addleaf($1),$2); }
	;

M	: ZERO			{ $$ = addnodeM(addleaf('0'),NULL); }
	| ZERO M		{ $$ = addnodeM(addleaf('0'),$2); }
	| ONE			{ $$ = addnodeM(addleaf('1'),NULL); }
	| ONE M			{ $$ = addnodeM(addleaf('1'),$2); }
	| DIG			{ $$ = addnodeM(addleaf($1),NULL); }
	| DIG M			{ $$ = addnodeM(addleaf($1),$2); }
	;

S	: P EOL			{ PT = $$ = addnodeS('\0',$1); }
	| PLUS P EOL		{ PT = $$ = addnodeS('+',$2); }
	| MINUS P EOL		{ PT = $$ = addnodeS('-',$2); }
	;

P	: T			{ $$ = addnodeP('\0',$1,NULL); }
	| T PLUS P		{ $$ = addnodeP('+',$1,$3); }
	| T MINUS P		{ $$ = addnodeP('-',$1,$3); }
	;

T	: ONE			{ $$ = addnodeT('N',addleaf('1'),NULL); }
	| N			{ $$ = addnodeT('N',$1,NULL); }
	| X			{ $$ = addnodeT('X',$1,NULL); }
	| N X			{ $$ = addnodeT('B',$1,$2); }
	;

X	: EX			{ $$ = addnodeX(addleaf('x'),NULL); }
	| EX PWR N		{ $$ = addnodeX(addleaf('x'),$3); }
	;

%%

void yyerror ( char *msg ) { fprintf(stderr, "Error: %s\n", msg); }
