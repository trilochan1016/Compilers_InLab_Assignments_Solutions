#include "lex.yy.c"
#include "y.tab.c"

node *addleaf ( char c )
{
   node *p;

   p = (node *)malloc(sizeof(node));
   p -> symb = c;
   if ((c >= '0') && (c <= '9')) p -> val = c - '0';
   p -> nc = 0;
   p -> C = NULL;
   return p;
}

node *addnodeN ( node *C1, node *C2 )
{
   node *p;

   p = (node *)malloc(sizeof(node));
   p -> symb = 'N';
   if (C2 != NULL) {
      p -> nc = 2;
      p -> C = (node **)malloc(2 * sizeof(node *));
      (p -> C)[0] = C1;
      (p -> C)[1] = C2;
   } else {
      p -> nc = 1;
      p -> C = (node **)malloc(sizeof(node *));
      (p -> C)[0] = C1;
   }
   return p;
}

node *addnodeM ( node *C1, node *C2 )
{
   node *p;

   p = (node *)malloc(sizeof(node));
   p -> symb = 'M';
   if (C2 != NULL) {
      p -> nc = 2;
      p -> C = (node **)malloc(2 * sizeof(node *));
      (p -> C)[0] = C1;
      (p -> C)[1] = C2;
   } else {
      p -> nc = 1;
      p -> C = (node **)malloc(sizeof(node *));
      (p -> C)[0] = C1;
   }
   return p;
}

node *addnodeS ( char c, node *P )
{
   node *p;

   p = (node *)malloc(sizeof(node));
   p -> symb = 'S';
   if (c == '\0') {
      p -> nc = 1;
      p -> C = (node **)malloc(sizeof(node *));
      (p -> C)[0] = P;
   } else if ((c == '+') || (c == '-')) {
      p -> nc = 2;
      p -> C = (node **)malloc(2 * sizeof(node *));
      (p -> C)[0] = addleaf(c);
      (p -> C)[1] = P;
   }
   return p;
}

node *addnodeP ( char c, node *C1, node *C2 )
{
   node *p;

   p = (node *)malloc(sizeof(node));
   p -> symb = 'P';
   if (c == '\0') {
      p -> nc = 1;
      p -> C = (node **)malloc(sizeof(node *));
      (p -> C)[0] = C1;
   } else if ((c == '+') || (c == '-')) {
      p -> nc = 3;
      p -> C = (node **)malloc(3 * sizeof(node *));
      (p -> C)[0] = C1;
      (p -> C)[1] = addleaf(c);
      (p -> C)[2] = C2;
   }
   return p;
}

node *addnodeT ( char c, node *C1, node *C2 )
{
   node *p;

   p = (node *)malloc(sizeof(node));
   p -> symb = 'T';

   if ((c == 'N') || (c == 'X')) {
      p -> nc = 1;
      p -> C = (node **)malloc(sizeof(node *));
      (p -> C)[0] = C1;
   } else if (c == 'B') {
      p -> nc = 2;
      p -> C = (node **)malloc(2 * sizeof(node *));
      (p -> C)[0] = C1;
      (p -> C)[1] = C2;
   }
   return p;
}

node *addnodeX ( node *C1, node *C2 )
{
   node *p;

   p = (node *)malloc(sizeof(node));
   p -> symb = 'X';

   if (C2 != NULL) {
      p -> nc = 3;
      p -> C = (node **)malloc(3 * sizeof(node *));
      (p -> C)[0] = C1;
      (p -> C)[1] = addleaf('^');
      (p -> C)[2] = C2;
   } else {
      p -> nc = 1;
      p -> C = (node **)malloc(sizeof(node *));
      (p -> C)[0] = C1;
   }
   return p;
}

void setatt ( node *p )
{
   switch (p -> symb) {
      case 'S':
         if (p -> nc == 1) {
            (p -> C)[0] -> inh = PLUS;
            setatt((p->C)[0]);
         } else {
            (p -> C)[1] -> inh = ((p -> C)[0] -> symb == '+') ? PLUS : MINUS;
            setatt((p->C)[1]);
         }
         break;
      case 'P':
         (p -> C)[0] -> inh = p -> inh;
         setatt((p->C)[0]);
         if (p -> nc == 3) {
            (p -> C)[2] -> inh = ((p -> C)[1] -> symb == '+') ? PLUS : MINUS;
            setatt((p->C)[2]);
         }
         break;
      case 'T':
         if (p -> nc == 1) {
            if ((p -> C)[0] -> symb == 'N') (p -> C)[0] -> inh = 0;
            setatt((p->C)[0]);
         } else {
            setatt((p->C)[0]);
            setatt((p->C)[1]);
         }
         break;
      case 'X':
         if (p -> nc == 3) {
            setatt((p->C)[2]);
         }
         break;
      case 'N':
         if (p -> nc == 1) {
            p -> val = (p -> C)[0] -> val;
         } else {
            (p -> C)[1] -> inh = (p -> C)[0] -> val;
            setatt((p->C)[1]);
            p -> val = (p -> C)[1] -> val;
         }
      case 'M':
         if (p -> nc == 1) {
            p -> val = 10 * (p -> inh) + (p -> C)[0] -> val;
         } else {
            (p -> C)[1] -> inh = 10 * (p -> inh) + (p -> C)[0] -> val;
            setatt((p->C)[1]);
            p -> val = (p -> C)[1] -> val;
         }
   }
}

void printPT ( node *p, int l )
{
   int i;

   for (i=0; i<l-1; ++i) printf("    ");
   if (l > 1) printf("==> "); else printf("    ");
   printf("%c [", p -> symb);
   if ((p -> symb == 'P') || (p -> symb == 'T'))
      printf("inh = %c", (p -> inh == PLUS) ? '+' : '-');
   else if ((p -> symb >= '0') && (p -> symb <= '9'))
      printf("val = %d", p -> val);
   else if (p -> symb == 'N')
      printf("val = %d", p -> val);
   else if (p -> symb == 'M')
      printf("inh = %d, val = %d", p -> inh, p -> val);
   printf("]\n");
   for (i=0; i<p->nc; ++i) printPT((p->C)[i],l+1);
}

int pwr ( int a, int e )
{
   int t;

   if (e == 0) return 1;
   t = pwr(a, e/2);
   t = t * t;
   if (e % 2 == 1) t *= a;
   return t;
}

int evalpoly ( node *p, int xval )
{
   if (p -> symb == 'S') {
      if (p -> nc == 1) return evalpoly((p -> C)[0], xval);
      else return evalpoly((p -> C)[1], xval);
   }
   if (p -> symb == 'P') {
      if (p -> nc == 1) return evalpoly((p -> C)[0], xval);
      else return evalpoly((p -> C)[0], xval) + evalpoly((p -> C)[2], xval);
   }
   if (p -> symb == 'T') {
      if (p -> inh == PLUS) {
         if (p -> nc == 1) return evalpoly((p -> C)[0], xval);
         else return evalpoly((p -> C)[0], xval) * evalpoly((p -> C)[1], xval);
      } else {
         if (p -> nc == 1) return -(evalpoly((p -> C)[0], xval));
         else return -(evalpoly((p -> C)[0], xval)) * evalpoly((p -> C)[1], xval);
      }
   }
   if (p -> symb == 'X') {
      if (p -> nc == 1) return xval;
      else return pwr(xval, evalpoly((p -> C)[2], xval));
   }
   if ((p -> symb == 'N') || (p -> symb == 'M')) return p -> val;
   if (p -> symb >= '1') return 1;
}

void prnterm ( int C, int E )
{
   if (C == 0) return;
   if (E == 0) {
      printf("%d", C);
   } else if (E == 1) {
      if (C != 1) printf("%d", C);
      printf("x");
   } else {
      if (C != 1) printf("%d", C);
      printf("x^%d", E);
   }
}

void printderivative ( node *p )
{
   node *q;

   if (p -> symb == 'S') {
      if (p -> nc == 1) printderivative((p -> C)[0]);
      else printderivative((p -> C)[1]);
   } else if (p -> symb == 'P') {
      if (p -> nc == 1) printderivative((p -> C)[0]);
      else {
         printderivative((p -> C)[0]);
         printderivative((p -> C)[2]);
      }
   } else if (p -> symb == 'T') {
      if (p -> nc == 1) {
         if ((p -> C)[0] -> symb == 'X') {
            if (p -> inh == PLUS) printf(" + "); else printf(" - ");
            if (((p -> C)[0]) -> nc == 1) prnterm(1,0);
            else prnterm(((p -> C)[0] -> C)[2] -> val, ((p -> C)[0] -> C)[2] -> val - 1);
         }
      } else {
         if (p -> inh == PLUS) printf(" + "); else printf(" - ");
         q = (p -> C)[1];
         if (q -> nc == 1) prnterm((p -> C)[0] -> val,0);
         else prnterm(((p -> C)[0] -> val) * ((q -> C)[2] -> val), (q -> C)[2] -> val - 1);
      }
   }
}

int main ( int argc, char *argv[] )
{
   int xval;

   yyparse();

   setatt(PT);

   printf("+++ The annotated parse tree is\n");
   printPT(PT,1);
   printf("\n");

   for (xval=-5; xval <=5; ++xval)
   printf("+++ f(%2d)  =  %10d\n", xval, evalpoly(PT,xval));

   printf("\n+++ f'(x)  = "); printderivative(PT); printf("\n");

   exit(0);
}
