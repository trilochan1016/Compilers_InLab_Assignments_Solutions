%{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int yylex ( );

typedef struct _addr {
   char loc;
   int ref;
} addr;

typedef struct {
   int op;
   char arg1loc;
   char arg2loc;
   char resloc;
   int arg1ref;
   int arg2ref;
   int resref;
} quad;
quad QT[65536];

typedef struct _place {
   char loc;
   int ref;
   struct _place *next;
} place;

typedef struct {
   char *name;
   place *storage;
} stentry;
stentry ST[1024];
int nsymb = 0;

int nextinst = 1;

int tmpno = 0;

addr *storeinst ( ) ;
void backpatch ( addr *, int ) ;
int STref ( char *, int ) ;
addr *iden2addr ( char * ) ;
addr *numb2addr ( int ) ;
addr *addboolcode ( int, addr *, addr * ) ;
addr *addtempcode ( int, addr *, addr * ) ;
void addasgncode ( addr *, addr * ) ;
void addloopcode ( addr * ) ;
%}

%start LIST

%union { struct _addr *ADDR; int OPCODE; int PUNC; int KWD; char *TEXT; int VAL; }

%token <VAL> NUMB
%token <TEXT> IDEN
%token <OPCODE> ADD SUB MUL DIV REM EQ NE LT LE GT GE JMP EXIT
%token <PUNC> LP RP
%token <KWD> SET WHEN WHILE

%type <ADDR> LIST STMT ASGN COND LOOP EXPR BOOL ATOM M
%type <OPCODE> OPER RELN

%%

LIST	: STMT				{ }
	| STMT LIST			{ }
	;

STMT	: ASGN				{ $$ = $1; }
	| COND				{ $$ = $1; }
	| LOOP				{ $$ = $1; }
	;

ASGN	: LP SET IDEN ATOM RP		{ addasgncode(iden2addr($3),$4); }
	;

COND	: LP WHEN BOOL LIST RP		{ backpatch($3,nextinst); }
	;

LOOP	: LP WHILE M BOOL LIST RP	{ addloopcode($3); backpatch($4,nextinst); }
	;

EXPR	: LP OPER ATOM ATOM RP		{ $$ = addtempcode($2,$3,$4); }
	;

BOOL	: LP RELN ATOM ATOM RP		{ $$ = addboolcode($2,$3,$4); }
	;

ATOM	: IDEN				{ $$ = iden2addr($1); }
	| NUMB				{ $$ = numb2addr($1); }
	| EXPR				{ $$ = $1; }
	;

OPER	: ADD				{ $$ = $1; }
	| SUB				{ $$ = $1; }
	| MUL				{ $$ = $1; }
	| DIV				{ $$ = $1; }
	| REM				{ $$ = $1; }
	;

RELN	: EQ				{ $$ = $1; }
	| NE				{ $$ = $1; }
	| LT				{ $$ = $1; }
	| LE				{ $$ = $1; }
	| GT				{ $$ = $1; }
	| GE				{ $$ = $1; }
	;

M	:				{ $$ = storeinst(); }
	;

%%

void yyerror ( char *msg ) { fprintf(stderr, "Error: %s\n", msg); }

addr *storeinst ( )
{
   addr *A;

   A = (addr *)malloc(sizeof(addr));
   A -> ref = nextinst;
   return A;
}

void backpatch ( addr *A, int i )
{
   QT[A -> ref].resref = i;
}

int STfind ( char *var )
{
   int i;
   for (i=0; i<nsymb; ++i) 
      if (!strcmp(ST[i].name, var)) return i;
   return -1;
}

int STref ( char *var, int istemp )
{
   int i;

   for (i=0; i<nsymb; ++i) {
      if (!strcmp(ST[i].name,var)) return i;
   }
   ST[nsymb].name = (char *)malloc((1 + strlen(var)) * sizeof(char));
   strcpy(ST[nsymb].name, var);
   if (istemp) {
      ST[nsymb].storage = NULL;
   } else {
      ST[nsymb].storage = (place *)malloc(sizeof(place));
      ST[nsymb].storage -> loc = 'M';
      ST[nsymb].storage -> ref = 4 * nsymb;
      ST[nsymb].storage -> next = NULL;
   }
   ++nsymb;
   return i;
}

addr *iden2addr ( char *var )
{
   addr *A;

   A = (addr *)malloc(sizeof(addr));
   A -> loc = 'M';
   A -> ref = STref(var,0);
   return A;
}

addr *numb2addr ( int val )
{
   addr *A;

   A = (addr *)malloc(sizeof(addr));
   A -> loc = 'I';
   A -> ref = val;
   return A;
}

addr *addboolcode ( int rel, addr *E1, addr *E2 )
{
   addr *A;

   A = (addr *)malloc(sizeof(addr));
   A -> loc = 'Q';
   A -> ref = nextinst;
   QT[nextinst].op = rel;
   QT[nextinst].arg1loc = E1 -> loc;
   QT[nextinst].arg1ref = E1 -> ref;
   QT[nextinst].arg2loc = E2 -> loc;
   QT[nextinst].arg2ref = E2 -> ref;
   QT[nextinst].resloc = 'J';
   QT[nextinst].resref = -1;
   ++nextinst;
   return A;
}

addr *addtempcode ( int op, addr *E1, addr *E2 )
{
   addr *A;
   char name[16];

   ++tmpno;
   A = (addr *)malloc(sizeof(addr));
   A -> loc = 'T';
   A -> ref = tmpno;
   QT[nextinst].op = op;
   QT[nextinst].arg1loc = E1 -> loc;
   QT[nextinst].arg1ref = E1 -> ref;
   QT[nextinst].arg2loc = E2 -> loc;
   QT[nextinst].arg2ref = E2 -> ref;
   QT[nextinst].resloc = A -> loc;
   QT[nextinst].resref = A -> ref;
   ++nextinst;
   sprintf(name, "$%d", A -> ref);
   STref(name, 1);
   return A;
}

void addasgncode ( addr *L, addr *R )
{
   QT[nextinst].op = SET;
   QT[nextinst].arg1loc = R -> loc;
   QT[nextinst].arg1ref = R -> ref;
   QT[nextinst].resloc = L -> loc;
   QT[nextinst].resref = L -> ref;
   ++nextinst;
}

void addloopcode ( addr *M )
{
   QT[nextinst].op = JMP;
   QT[nextinst].resloc = 'J';
   QT[nextinst].resref = M -> ref;
   ++nextinst;
}
