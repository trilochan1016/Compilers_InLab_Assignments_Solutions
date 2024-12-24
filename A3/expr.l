%{
#include "y.tab.h"
int yylex();
void yyerror(char *);
%}

ws	[ \t\n]+
num	[+-]?[0-9]+
id	[a-zA-Z_][a-zA-Z0-9_]*

%%

{ws}	{ }
"set"	{ yylval.kwd = SET; return SET; }
{num}	{ yylval.text = (char *)malloc((1 + strlen(yytext))*sizeof(char)); strcpy(yylval.text, yytext); return NUM; }
{id}	{ yylval.text = (char *)malloc((1 + strlen(yytext))*sizeof(char)); strcpy(yylval.text, yytext); return ID; }
"+"	{ yylval.opcode = ADD; return ADD; }
"-"	{ yylval.opcode = SUB; return SUB; }
"*"	{ yylval.opcode = MUL; return MUL; }
"/"	{ yylval.opcode = DIV; return DIV; }
"%"	{ yylval.opcode = REM; return REM; }
"**"	{ yylval.opcode = POW; return POW; }
"("	{ yylval.punc = LP; return LP; }
")"	{ yylval.punc = RP; return RP; }
.	{ fprintf(stderr, "Error: Invalid character \"%s\"\n", yytext); }

%%

int yywrap ( ) { return 1; }

