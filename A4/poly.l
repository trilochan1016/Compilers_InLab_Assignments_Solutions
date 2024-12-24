%{
#include "y.tab.h"
int yylex();
void yyerror(char *);
%}

ws	[ \t]+
eol	\n
dig	[2-9]

%%

{ws}	{ }
{dig}	{ yylval.symb = yytext[0]; return DIG; }
"1"	{ yylval.symb = '1'; return ONE; }
"0"	{ yylval.symb = '0'; return ZERO; }
"x"	{ yylval.symb = yytext[0]; return EX; }
"^"	{ yylval.symb = yytext[0]; return PWR; }
"+"	{ yylval.symb = yytext[0]; return PLUS; }
"-"	{ yylval.symb = yytext[0]; return MINUS; }
{eol}	{ yylval.symb = yytext[0]; return EOL; }
.	{ fprintf(stderr, "Invalid symbol %s\n", yytext); }

%%

int yywrap ( ) { return 1; }
