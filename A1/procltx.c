#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lex.yy.c"

typedef struct _node {
   char *name;
   int nocc;
   struct _node *next;
} node;
typedef node *nametable;

nametable addtbl ( nametable T, char *id )
{
   node *p;

   p = T;
   while (p) {
      if (!strcmp(p->name,id)) {
         ++(p -> nocc);
         return T;
      }
      p = p -> next;
   }
   p = (node *)malloc(sizeof(node));
   p -> name = (char *)malloc((strlen(id)+1) * sizeof(char));
   strcpy(p -> name, id);
   p -> nocc = 1;
   p -> next = T;
   return p;
}

nametable addenv ( nametable T, char *id )
{
   char *s, *e;

   s = strchr(id, '{');
   e = strchr(id, '}');
   *e = '\0';
   return addtbl(T,s+1);
}

void prnnames ( nametable T )
{
   if (T == NULL) return;
   prnnames(T -> next);
   printf("\t%s (%d)\n", T -> name, T -> nocc);
}

int main ()
{
   int nexttok;
   int nmth = 0, ndsp = 0;
   nametable CT = NULL, ET = NULL;

   while ((nexttok = yylex())) {
      switch (nexttok) {
         case MTH: nmth += mth; break;
         case DSP: ndsp += dsp; break;
         case ENV: if (env) ET = addenv(ET,yytext); break;
         case CMD: CT = addtbl(CT,yytext); break;
         default: printf("Unknown token\n"); break;
      }
   }

   printf("Commands used:\n"); prnnames(CT);
   printf("Environments used:\n"); prnnames(ET);
   printf("%d math equations found\n", nmth);
   printf("%d displayed equations found\n", ndsp);

   exit(0);
}