#include "lex.yy.c"
#include "y.tab.c"

int addsymbol ( int type, char *text )
{
   int i;

   for (i=0; i<nsymb; ++i) if (!strcmp(ST[i].data,text)) return i;
   i = nsymb;
   ++nsymb;
   ST[i].type = type;
   ST[i].data = (char *)malloc((1 + strlen(text))*sizeof(char));
   strcpy(ST[i].data,text);
   if (type == ID) {
      ST[i].value = 0;
      ST[i].init = 0;
   } else if (type == NUM) {
      ST[i].value = atoi(text);
      ST[i].init = 1;
   } else {
      fprintf(stderr, "Error: Unknown type %d for symbol table\n", type);
      --nsymb;
      return -1;
   }
   return i;
}

int setsymbol ( int i, int val )
{
   if ((i < 0) || (i >= nsymb)) {
      fprintf(stderr, "Error: Invalid index %d in symbol table\n", i);
      return -1;
   }
   if (ST[i].type != ID) {
      fprintf(stderr, "Error: Attempt to set non-ID in symbol table\n");
      return -1;
   }
   ST[i].value = val;
   ST[i].init = 1;
   return i;
}

int getsymbol ( char *id )
{
   int i;

   for (i=0; i<nsymb; ++i) {
      if (!strcmp(ST[i].data, id)) {
         if (ST[i].init == 0) {
            fprintf(stderr, "Error: Attempt to read uninitialized value\n");
            return 0;
         }
         return ST[i].value;
      }
   }
   fprintf(stderr, "Error: Invalid ID %s\n", id);
   return 0;
}

int getvalue ( int i )
{
   if ((i < 0) || (i >= nsymb)) {
      fprintf(stderr, "Error: Unknown index %d in sylbol table\n", i);
      return 0;
   }
   if (ST[i].init == 0) {
      fprintf(stderr, "Error: Attempt to read uninitialized value\n");
      return 0;
   }
   return ST[i].value;
}

node *addleaf ( int i )
{
   node *p;

   if ((i < 0) || (i >= nsymb)) {
      fprintf(stderr, "Invalid index %d in symbol table\n", i);
      return NULL;
   }
   p = (node *)malloc(sizeof(node));
   p -> type = ST[i].type;
   p -> data = i;
   p -> L = p -> R = NULL;
   return p;
}

node *addnode ( int op, node *left, node *right )
{
   node *p;

   p = (node *)malloc(sizeof(node));
   p -> type = OP;
   p -> data = op;
   p -> L = left;
   p -> R = right;
   return p;
}

int intpow ( int a, int e )
{
   int prod = 1, a2pow = a;
   if (e < 0) return 0;
   while (e > 0) {
      if (e % 2) prod *= a2pow;
      e /= 2;
      a2pow *= a2pow;
   }
   return prod;
}

int evaltree ( node *p )
{
   int val, lval, rval;

   if (p == NULL) {
      fprintf(stderr, "Error: Empty expression tree\n");
      return 0;
   }
   if ( (p -> type == ID) || (p -> type == NUM) ) {
      val = getvalue(p -> data);
   } else if (p -> type == OP) {
      lval = evaltree(p -> L);
      rval = evaltree(p -> R);
      switch (p -> data) {
         case ADD: val = lval + rval; break;
         case SUB: val = lval - rval; break;
         case MUL: val = lval * rval; break;
         case DIV: val = lval / rval; break;
         case REM: val = lval % rval; break;
         case POW: val = intpow(lval,rval); break;
         default:
            fprintf(stderr, "Error: Unknown opcode %d\n", p -> data);
            val = 0;
            break;
      }
   } else {
      fprintf(stderr, "Error: Unknown node type in expression tree\n");
      val = 0;
   }
   free(p);
   return val;
}

int main ()
{
   yyparse();
   exit(0);
}
