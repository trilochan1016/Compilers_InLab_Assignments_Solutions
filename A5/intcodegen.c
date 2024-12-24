#include "lex.yy.c"
#include "y.tab.c"

void preamble ( )
{
   int i;

   fp = (FILE *)fopen("intcode.c","w");
   fprintf(fp, "#include <stdio.h>\n#include <stdlib.h>\n#include \"auxi.c\"\n\n");
   fprintf(fp, "int main ( )\n");
   fprintf(fp, "{\n   int R[12];\n   int MEM[65536];\n\n");
   for (i=0; i<12; ++i) RS[i] = 0;
}

void postamble ( )
{
   fprintf(fp, "\n   exit(0);\n}\n");
   fclose(fp);
   fp = NULL;
}

int storeID ( char *id )
{
   int i, j;

   for (i=0; i<nsymb; ++i) {
      if (!strcmp(ST[i].name, id)) {
         j = ST[i].offset;
         return j;
      }
   }
   ST[nsymb].name = (char *)malloc((1 + strlen(id)) * sizeof(char));
   strcpy(ST[nsymb].name, id);
   j = ST[nsymb].offset = OFFSET;
   ++OFFSET;
   ++nsymb;
   return j;
}

struct addr *fetchmem ( struct addr *memaddr )
{
   struct addr *regaddr;

   regaddr = (struct addr *)malloc(sizeof(struct addr));
   regaddr -> loc = 'R';
   if (RS[0] == 0) {
      regaddr -> ref = 0;
      RS[0] = 1;
   } else {
      regaddr -> ref = 1;
      RS[1] = 1;
   }
   return regaddr;
}

void freeregs ( )
{
   int i;

   for (i=0; i<12; ++i) RS[i] = 0;
}

struct addr *NUMtoADDR ( int num )
{
   struct addr *ADDR;

   ADDR = (struct addr *)malloc(sizeof(struct addr));
   ADDR -> loc = 'I';
   ADDR -> ref = num;
   return ADDR;
}

struct addr *IDtoADDR ( char *id )
{
   struct addr *ADDR;

   ADDR = (struct addr *)malloc(sizeof(struct addr));
   ADDR -> loc = 'M';
   ADDR -> ref = storeID(id);
   return ADDR;
}

void setID ( struct addr *lval, struct addr *rval )
{
   struct addr *RVAL;

   if (rval -> loc == 'M') {
      RVAL = fetchmem(rval);
      fprintf(fp, "   R[%d] = MEM[%d];\n", RVAL -> ref, rval -> ref);
      fprintf(fp, "   MEM[%d] = R[%d];\n", lval -> ref, RVAL -> ref);
      RS[0] = RS[1] = 0;
   } else if (rval -> loc == 'R') {
      fprintf(fp, "   MEM[%d] = R[%d];\n", lval -> ref, rval -> ref);
      RS[rval -> ref] = 0;
   } else if (rval -> loc == 'I') {
      fprintf(fp, "   MEM[%d] = %d;\n", lval -> ref, rval -> ref);
   }
   fprintf(fp, "   mprn(MEM,%d);\n", lval -> ref);
}

struct addr *makeop ( int op, struct addr *arg1, struct addr *arg2 )
{
   int i;
   struct addr *A1, *A2, *RES;
   char tmpname[16];

   if (arg1 -> loc == 'M') {
      A1 = fetchmem(arg1);
      fprintf(fp, "   R[%d] = MEM[%d];\n", A1 -> ref, arg1 -> ref);
   } else {
      A1 = arg1;
      if (arg1 -> loc == 'R') RS[arg1 -> ref] = 0;
   }

   if (arg2 -> loc == 'M') {
      A2 = fetchmem(arg2);
      fprintf(fp, "   R[%d] = MEM[%d];\n", A2 -> ref, arg2 -> ref);
   } else {
      A2 = arg2;
      if (arg2 -> loc == 'R') RS[arg2 -> ref] = 0;
   }

   RES = (struct addr *)malloc(sizeof(struct addr));
   ++TEMPNO;
   for (i=2; i<12; ++i) {
      if (RS[i] == 0) {
         RES -> loc = 'R';
         RES -> ref = i;
         RS[i] = 1;
         fprintf(fp, "   R[%d] = ", i);
         break;
      }
   }
   if (i == 12) {
      RES -> loc = 'M';
      sprintf(tmpname, "$%d", TEMPNO);
      RES -> ref = storeID(tmpname);
      fprintf(fp, "   R[%d] = ", 0);
   }

   switch (op) {
      case ADD:
         if (A1 -> loc == 'I') fprintf(fp, "%d", A1 -> ref);
         else fprintf(fp, "R[%d]", A1 -> ref);
         fprintf(fp, " + ");
         if (A2 -> loc == 'I') fprintf(fp, "%d;\n", A2 -> ref);
         else fprintf(fp, "R[%d];\n", A2 -> ref);
         break;
      case SUB:
         if (A1 -> loc == 'I') fprintf(fp, "%d", A1 -> ref);
         else fprintf(fp, "R[%d]", A1 -> ref);
         fprintf(fp, " - ");
         if (A2 -> loc == 'I') fprintf(fp, "%d;\n", A2 -> ref);
         else fprintf(fp, "R[%d];\n", A2 -> ref);
         break;
      case MUL:
         if (A1 -> loc == 'I') fprintf(fp, "%d", A1 -> ref);
         else fprintf(fp, "R[%d]", A1 -> ref);
         fprintf(fp, " * ");
         if (A2 -> loc == 'I') fprintf(fp, "%d;\n", A2 -> ref);
         else fprintf(fp, "R[%d];\n", A2 -> ref);
         break;
      case DIV:
         if (A1 -> loc == 'I') fprintf(fp, "%d", A1 -> ref);
         else fprintf(fp, "R[%d]", A1 -> ref);
         fprintf(fp, " / ");
         if (A2 -> loc == 'I') fprintf(fp, "%d;\n", A2 -> ref);
         else fprintf(fp, "R[%d];\n", A2 -> ref);
         break;
      case REM:
         if (A1 -> loc == 'I') fprintf(fp, "%d", A1 -> ref);
         else fprintf(fp, "R[%d]", A1 -> ref);
         fprintf(fp, " %% ");
         if (A2 -> loc == 'I') fprintf(fp, "%d;\n", A2 -> ref);
         else fprintf(fp, "R[%d];\n", A2 -> ref);
         break;
      case PWR:
         fprintf(fp, "pwr(");
         if (A1 -> loc == 'I') fprintf(fp, "%d", A1 -> ref);
         else fprintf(fp, "R[%d]", A1 -> ref);
         fprintf(fp, ",");
         if (A2 -> loc == 'I') fprintf(fp, "%d", A2 -> ref);
         else fprintf(fp, "R[%d]", A2 -> ref);
         fprintf(fp, ");\n");
         break;
   }
   if (RES -> loc == 'M') fprintf(fp, "   MEM[%d] = R[0];\n", RES -> ref);
   RS[0] = RS[1] = 0;
   return RES;
}

void prnexpr ( struct addr *E )
{
   if (E -> loc == 'M') {
      fprintf(fp, "   eprn(MEM,%d);\n", E -> ref);
   } else if (E -> loc == 'R') {
      fprintf(fp, "   eprn(R,%d);\n", E -> ref);
   } else if (E -> loc == 'I') {
      fprintf(fp, "   eprn(NULL,%d);\n", E -> ref);
   }
   freeregs();
}

int main ( )
{
   preamble();
   yyparse();
   postamble();

   exit(0);
}
