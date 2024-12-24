%{
#define EXPR 1001
#define OP 1002
#define ARG 1003
#define ID 1004
#define NUM 1005
#define LP 1006
#define RP 1007
#define ADD 1008
#define SUB 1009
#define MUL 1010
#define DIV 1011
#define REM 1012
#define INV 1013
%}

ws	[ \t\n]+
id	[a-zA-Z_][a-zA-Z0-9_]*
num	[+-]?[0-9]+
badop	[^a-zA-Z0-9() \t\n]+

%%

{ws}	{ }
{id}	{ return ID; }
{num}	{ return NUM; }
"("	{ return LP; }
")"	{ return RP; }
"+"	{ return ADD; }
"-"	{ return SUB; }
"*"	{ return MUL; }
"/"	{ return DIV; }
"%"	{ return REM; }
{badop}	{ return INV; }

%%
