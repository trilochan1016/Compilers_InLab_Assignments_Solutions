#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.yy.c"

#define binop(x) ( (x == ADD) || (x == SUB) || (x == MUL) || (x == DIV) || (x == REM) )

typedef struct _stnode {
   char *id;
   int value;
   struct _stnode *next;
} stnode;
typedef stnode *symbtbl;

typedef struct _ctnode {
   int value;
   struct _ctnode *next;
} ctnode;
typedef ctnode *constbl;

typedef struct _stacknode {
   int item;
   struct _stacknode *next;
} stacknode;
typedef stacknode *stack;

typedef struct _treenode {
   int type;
   union {
      char *op;
      stnode *id;
      ctnode *num;
   } ref;
   struct _treenode *L, *R, *P;
} treenode;
typedef treenode *parsetree;

symbtbl insertid ( symbtbl T, char *id, stnode **ref )
{
   stnode *p;

   p = T;
   while (p != NULL) {
      if (!strcmp(p->id,id)) {
         *ref = p;
         return T;
      }
      p = p -> next;
   }
   p = (stnode *)malloc(sizeof(stnode));
   p -> id = (char *)malloc((1 + strlen(id)) * sizeof(char));
   strcpy(p -> id, id);
   p -> value = 0;
   p -> next = T;
   *ref = p;
   return p;
}

constbl insertnum ( constbl C, char *num, ctnode **ref )
{
   ctnode *p;

   p = (ctnode *)malloc(sizeof(ctnode));
   p -> value = atoi(num);
   p -> next = C;
   *ref = p;
   return p;
}

int top ( stack S )
{
   if (S == NULL) {
      fprintf(stderr, "*** Read from an empty stack\n");
      exit(1);
   }
   return S -> item;
}

stack push ( stack S, int item )
{
   stacknode *p;

   p = (stacknode *)malloc(sizeof(stnode));
   p -> item = item;
   p -> next = S;
   return p;
}

stack pop ( stack S )
{
   stacknode *p;

   if (S == NULL) {
      fprintf(stderr, "*** Pop from an empty stack\n");
      exit(1);
   }
   p = S;
   S = S -> next;
   free(p);
   return S;
}

/* The following function parse() implements the textbook predictive-parsing algorithm
   under an LL(1) grammar.

   The M table for this grammar has only the following non-error entries:
   M[EXPR,LP] = EXPR -> ( OP ARG ARG )

   M[OP,binop] = OP -> binop for binop in { ADD, SUB, MUL, DIV, REM }

   M[ARG,ID] = ARG -> ID
   M[ARG,NUM] = ARG -> NUM
   M[ARG,LP] = ARG -> ( EXPR )
*/

parsetree parse ( symbtbl *T, constbl *C )
{
   int A, a;
   stack S = NULL;
   parsetree PT = NULL;
   treenode *p = NULL, *q;

   S = push(S,EXPR);
   a = yylex(); // printf("Next token is %s\n", yytext);
   while (1) {
      if (S == NULL) break;
      A = top(S); S = pop(S);
      if (A == EXPR) {
         if (a != LP) {
            fprintf(stderr, "*** Error: LP expected in place of %s\n", yytext);
            exit(2);
         }
         S = push(S,RP);
         S = push(S,ARG);
         S = push(S,ARG);
         S = push(S,OP);
         S = push(S,LP);
      } else if (A == ARG) {
         if (a == ID) S = push(S,ID);
         else if (a == NUM) S = push(S,NUM);
         else if (a == LP) S = push(S,EXPR);
         else {
            fprintf(stderr, "*** Error: ID/NUM/LP expected in place of %s\n", yytext);
            exit(3);
         }
      } else if (A == OP) {
         if(!binop(a)) {
            fprintf(stderr, "*** Error: Invalid operator %s found\n", yytext);
            exit(4);
         }
         S = push(S,a);
      } else {
         /* A is a terminal symbol. So a must be the same as A */
         if (A == ID) {
            /* Create a new ID node under the current parent p */
            if (a != ID) {
               fprintf(stderr, "*** ERROR: ID expected in place of %s\n", yytext);
               exit(5);
            }
            if ( (p == NULL) || ( (p -> L != NULL) && (p -> R != NULL) ) ) {
               fprintf(stderr, "*** Error: Invalid id %s found\n", yytext);
               exit(6);
            }
            if (p -> L == NULL) q = p -> L = (treenode *)malloc(sizeof(treenode));
            else q = p -> R = (treenode *)malloc(sizeof(treenode));
            q -> type = ID;
            *T = insertid(*T,yytext,&((q->ref).id));
            q -> L = q -> R = NULL;
            q -> P = p;
         } else if (A == NUM) {
            /* Create a new NUM node under the current parent p */
            if (a != NUM) {
               fprintf(stderr, "*** ERROR: Number expected in place of %s\n", yytext);
               exit(7);
            }
            if ( (p == NULL) || ( (p -> L != NULL) && (p -> R != NULL) ) ) {
               fprintf(stderr, "*** Error: Invalid number %s found\n", yytext);
               exit(8);
            }
            if (p -> L == NULL) q = p -> L = (treenode *)malloc(sizeof(treenode));
            else q = p -> R = (treenode *)malloc(sizeof(treenode));
            q -> type = NUM;
            *C = insertnum(*C,yytext,&((q->ref).num));
            q -> L = q -> R = NULL;
            q -> P = p;
         } else if (A == LP) {
            if (a != LP) {
               fprintf(stderr, "*** Error: Left parenthesis expected in place of %s\n", yytext);
               exit(9);
            }
         } else if (A == RP) {
            /* Move up until a node is found without its right child (or the root is reached) */
            if (a != RP) {
               fprintf(stderr, "*** Error: Right parenthesis expected in place of %s\n", yytext);
               exit(10);
            }
            while ((p != PT) && (p -> R != NULL)) p = p -> P;
         } else if (binop(A)) {
            /* Create a new operator node under the current parent p,
               and update the current parent p to this new operator node */
            if (PT == NULL) {
               p = PT = (treenode *)malloc(sizeof(treenode));
               PT -> P = NULL;
            } else {
               if (p -> L == NULL) q = p -> L = (treenode *)malloc(sizeof(treenode));
               else if (p -> R == NULL) q = p -> R = (treenode *)malloc(sizeof(treenode));
               else {
                  fprintf(stderr, "*** Error: Cannot create subtree for operator %s (%d)\n", yytext, p -> type);
                  exit(11);
               }
               q -> P = p;
               p = q;
            }
            p -> type = OP;
            (p -> ref).op = (char *)malloc((1 + strlen(yytext)) * sizeof(char));
            strcpy((p -> ref).op, yytext);
            p -> L = p -> R = NULL;
         }
         /* Here a is consumed from input, so set the next input token (if any) as a */
         if (S != NULL) a = yylex(); // printf("Next token is %s\n", yytext);
      }
   }
   if (S != NULL) {
      fprintf(stderr, "*** Error: Extra characters in input\n");
      exit(12);
   }
   printf("Parsing is successful\n");
   return PT;
}

void prntree ( parsetree PT, int level )
{
   int l;

   if (PT == NULL) return;
   for (l=0; l<level-1; ++l) printf("      ");
   if (level) printf(" ---> ");
   if (PT -> type == OP) printf("OP(%s)", (PT -> ref).op);
   else if (PT -> type == ID) printf("ID(%s)", (PT -> ref).id -> id);
   else if (PT -> type == NUM) printf("NUM(%d)", (PT -> ref).num -> value);
   printf("\n");
   prntree(PT -> L, level + 1);
   prntree(PT -> R, level + 1);
}

void prnsymbols ( symbtbl T )
{
   while (T) {
      printf("\"%s\"\n", T -> id);
      T = T -> next;
   }
}

void readvalues ( symbtbl T )
{
   int tok;

   if (T == NULL) return;
   readvalues(T -> next);
   tok = yylex();
   if (tok != NUM) {
      fprintf(stderr, "*** Error: %s is not an integer value for %s\n", yytext, T -> id);
      exit(13);
   }
   T -> value = atoi(yytext);
   printf("%s = %d\n", T -> id, T -> value);
}

int evaltree ( parsetree PT )
{
   if (PT == NULL) return 0;
   switch (PT -> type) {
      case ID: return ((PT -> ref).id) -> value;
      case NUM: return ((PT -> ref).num) -> value;
      case OP:
         if (!strcmp((PT -> ref).op,"+")) return evaltree(PT -> L) + evaltree(PT -> R);
         if (!strcmp((PT -> ref).op,"-")) return evaltree(PT -> L) - evaltree(PT -> R);
         if (!strcmp((PT -> ref).op,"*")) return evaltree(PT -> L) * evaltree(PT -> R);
         if (!strcmp((PT -> ref).op,"/")) return evaltree(PT -> L) / evaltree(PT -> R);
         if (!strcmp((PT -> ref).op,"%")) return evaltree(PT -> L) % evaltree(PT -> R);
   }
}

int main ( )
{
   parsetree PT;
   symbtbl T = NULL;
   constbl C = NULL;

   PT = parse(&T, &C);
   prntree(PT,0);

   if (T) {
      printf("Reading variable values from the input\n");
      readvalues(T);
   }

   printf("The expression evaluates to %d\n", evaltree(PT));

   exit(0);
}
