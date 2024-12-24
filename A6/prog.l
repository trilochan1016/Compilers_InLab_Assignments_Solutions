%{
#include "y.tab.h"
int yylex ( ) ;
void yyerror (char * ) ;
%}

ws      [ \t\n]+
num     [+-]?[0-9]+
id      [a-zA-Z_][a-zA-Z0-9_]*
while	loop{ws}while

%%

{ws}	{ }
"set"	{ yylval.KWD = SET; return SET; }
"when"	{ yylval.KWD = WHEN; return WHEN; }
{while}	{ yylval.KWD = WHILE; return WHILE; }
{num}	{ yylval.VAL = atoi(yytext); return NUMB; }
{id}	{ yylval.TEXT = (char *)malloc((1 + strlen(yytext))*sizeof(char));
          strcpy(yylval.TEXT, yytext);
          return IDEN; }
"="	{ yylval.OPCODE = EQ; return EQ; }
"/="	{ yylval.OPCODE = NE; return NE; }
"<="	{ yylval.OPCODE = LE; return LE; }
"<"	{ yylval.OPCODE = LT; return LT; }
">="	{ yylval.OPCODE = GE; return GE; }
">"	{ yylval.OPCODE = GT; return GT; }
"+"	{ yylval.OPCODE = ADD; return ADD; }
"-"	{ yylval.OPCODE = SUB; return SUB; }
"*"	{ yylval.OPCODE = MUL; return MUL; }
"/"	{ yylval.OPCODE = DIV; return DIV; }
"%"	{ yylval.OPCODE = REM; return REM; }
"("	{ yylval.PUNC = LP; return LP; }
")"	{ yylval.PUNC = RP; return RP; }
.	{ fprintf(stderr, "Error: Invalid character \"%s\"\n", yytext); }

%%

int yywrap ( ) { return 1; }
