#include "lex.yy.c"
#include "y.tab.c"

#define LOD 1001
#define LDI 1002
#define STR 1003
#define JEQ 1004
#define JNE 1005
#define JLT 1006
#define JLE 1007
#define JGT 1008
#define JGE 1009

quad tcode[8192];
int tnextinst = 1;

int leader[8192];
int leaderinst[8192];
int nblock = 0;

int nreg = 5;
place **R = NULL;

typedef struct {
   int R1;
   int R2;
   int R;
} regs;

void printop ( int op )
{
   switch (op) {
      case ADD: printf(" + "); break;
      case SUB: printf(" - "); break;
      case MUL: printf(" * "); break;
      case DIV: printf(" / "); break;
      case REM: printf(" %% "); break;
      case EQ: printf(" == "); break;
      case NE: printf(" != "); break;
      case LT: printf(" < "); break;
      case LE: printf(" <= "); break;
      case GT: printf(" > "); break;
      case GE: printf(" >= "); break;
      case SET: printf(" SET "); break;
      default: printf(" ??? "); break;
   }
}

void printarg ( char loc, int ref )
{
   if (loc == 'M') printf("%s", ST[ref].name);
   else if (loc == 'T') printf("$%d", ref);
   else if (loc == 'I') printf("%d", ref);
   else printf("UNKNOWN");
}

int isop ( int op )
{
   return ((op == ADD) || (op == SUB) || (op == MUL) || (op == DIV) || (op == REM));
}

int isrel ( int rel )
{
   return ((rel == EQ) || (rel == NE) || (rel == LT) || (rel == LE) || (rel == GT) || (rel == GE));
}

void genleaders ( )
{
   int i, isleader[8192];

   isleader[1] = 1;
   for (i=2; i<=nextinst; ++i) isleader[i] = 0;
   isleader[nextinst] = 1;
   for (i=1; i<=nextinst; ++i) {
      if (isrel(QT[i].op) || (QT[i].op == JMP)) {
         isleader[i+1] = 1;
         isleader[QT[i].resref] = 1;
      } else if (QT[i].op == EXIT) {
         isleader[i] = 1;
      }
   }
   for (i=1; i<=nextinst; ++i) {
      if (isleader[i]) {
         ++nblock;
         leader[nblock] = i;
      }
   }
}

void prnblocks ( )
{
   int i, j = 1;

   for (i=1; i<nextinst; ++i) {
      if (i == leader[j]) {
         printf("\nBlock %d\n", j);
         ++j;
      }
      printf("   %d\t: ", i);
      if (isop(QT[i].op)) {
         printarg(QT[i].resloc,QT[i].resref);
         printf(" = ");
         printarg(QT[i].arg1loc,QT[i].arg1ref);
         printop(QT[i].op);
         printarg(QT[i].arg2loc,QT[i].arg2ref);
      } else if (QT[i].op == SET) {
         printarg(QT[i].resloc,QT[i].resref);
         printf(" = ");
         printarg(QT[i].arg1loc,QT[i].arg1ref);
      } else if (isrel(QT[i].op)) {
         printf("iffalse (");
         printarg(QT[i].arg1loc,QT[i].arg1ref);
         printop(QT[i].op);
         printarg(QT[i].arg2loc,QT[i].arg2ref);
         printf(") goto %d", QT[i].resref);
      } else if (QT[i].op == JMP) {
         printf("goto %d", QT[i].resref);
      } else if (QT[i].op == EXIT) {
      }
      printf("\n");
   }
   printf("\n   %d\t:\n", i);
}

void initreg ( )
{
   int i;
   place *p, *q;

   if (R != NULL) {
      for (i=0; i<=nreg; ++i) {
         p = R[i];
         while (p != NULL) {
            q = p -> next;
            free(p);
            p = q;
         }
      }
      R = NULL;
   }
   R = (place **)malloc((nreg + 1) * sizeof(place *));
   for (i=0; i<=nreg; ++i) R[i] = NULL;
}

void loadreg ( int op, char loc, int ref, int r )
{
   int i;
   place *p, *q;
   char name[256];

   tcode[tnextinst].op = op;
   tcode[tnextinst].arg1loc = loc;
   tcode[tnextinst].arg1ref = ref;
   tcode[tnextinst].resloc = 'R';
   tcode[tnextinst].resref = r;
   ++tnextinst;

   if (op == LDI) return;

   p = R[r];
   while (p) {
      q = p -> next;
      free(p);
      p = q;
   }
   R[r] = NULL;

   R[r] = (place *)malloc(sizeof(place));
   R[r] -> loc = loc;
   R[r] -> ref = ref;
   R[r] -> next = NULL;

   if (loc == 'M') {
      i = ref;
   } else {
      sprintf(name, "$%d", ref);
      i = STfind(name);
   }

   p = (place *)malloc(sizeof(place));
   p -> loc = 'R';
   p -> ref = r;
   p -> next = ST[i].storage;
   ST[i].storage = p;
}

void regcont ( )
{
   place *p;
   int r;

   for (r=1; r<=nreg; ++r) {
      printf("R%d:", r);
      p = R[r];
      while (p) {
         printf(" (%c,%d)", p -> loc, p -> ref);
         p = p -> next;
      }
      printf("  ");
   }
   printf("\n");
}

int attempt1 ( char loc, int ref )
{
   int r;
   place *p;

   for (r=1; r<=nreg; ++r) {
      p = R[r];
      while (p) {
         if ((p -> loc == loc) && (p -> ref == ref))
            return r;
         p = p -> next;
      }
   }
   return -1;
}

int attempt2 ( char loc, int ref )
{
   int r;

   for (r=1; r<=nreg; ++r) {
      if (R[r] == NULL) return r;
   }
   return -1;
}

int attempt3 ( char loc, int ref )
{
   int r, i;
   place *p, *q;
   char name[16];

   for (r=1; r<=nreg; ++r) {
      p = R[r];
      while (p) {
         if (loc == 'T') {
            sprintf(name, "$%d", ref);
            i = STfind(name);
         } else {
            i = ref;
         }
         q = ST[i].storage;
         while (q) {
            if (q -> loc == 'M') break;
            q = q -> next;
         }
         if (q == NULL) break;
         p = p -> next;
      }
      if (p == NULL) return r;
   }
   return -1;
}

/* This is not the Attempt 4 of the algorithm in the book. The actual Attempt 4 is this.

   Operation: x = y + z
   Suppose that Register R stores x. Irrespective of whether x is now memory resident
   or not, it is going to be rewritten. If z != x, then use Rx = Ry = R. In this case,
   if R stores unsaved items to be used later, write them back to memory.
*/
int attempt4 ( char loc, int ref )
{
   place *p;
   int r;

   for (r=1; r<=nreg; ++r) {
      p = R[r];
      if ((p -> loc == loc) && (p -> ref == ref) && (p -> next == NULL))
         return r;
   }
   return -1;
}

int used ( char loc, int ref, int inst, int end )
{
   while (inst <= end) {
      if ( (isop(QT[inst].op)) || (isrel(QT[inst].op))) {
         if ((QT[inst].arg1loc == loc) && (QT[inst].arg1ref == ref)) return 1;
         if ((QT[inst].arg2loc == loc) && (QT[inst].arg2ref == ref)) return 1;
      } else if (QT[inst].op == SET) {
         if ((QT[inst].arg1loc == loc) && (QT[inst].arg1ref == ref)) return 1;
      }
      ++inst;
   }
   return 0;
}

int attempt5 ( char loc, int ref, int inst, int end )
{
   int r;
   place *p;

   for (r=1; r<=nreg; ++r) {
      p = R[r];
      while (p) {
         if (p -> loc == 'M') break;
         if (used(p->loc, p->ref, inst, end)) break;
         p = p -> next;
      }
      if (p == NULL) return r;
   }
   return -1;
}

void storereg ( char loc, int ref, int r, int i )
{
   place *p, *q;

   tcode[tnextinst].op = STR;
   tcode[tnextinst].arg1loc = 'R';
   tcode[tnextinst].arg1ref = r;
   tcode[tnextinst].resloc = loc;
   tcode[tnextinst].resref = ref;
   ++tnextinst;

   p = ST[i].storage;
   while (p) {
      q = p -> next;
      free(p);
      p = q;
   }

   ST[i].storage = (place *)malloc(sizeof(place));
   ST[i].storage -> loc = 'M';
   ST[i].storage -> ref = 4 * i;
   ST[i].storage -> next = NULL;
}

void delfromR ( char loc, int ref, int r )
{
   place *p, *q;

   if (R[r] == NULL) return;
   while ((R[r] != NULL) && (R[r] -> loc == loc) && (R[r] -> ref == ref)) {
      p = R[r];
      R[r] = p -> next;
      free(p);
   }
   if (R[r] == NULL) return;
   p = R[r];
   while (p -> next) {
      if ((p -> next -> loc == loc) && (p -> next -> ref == ref)) {
         q = p -> next;
         p -> next = q -> next;
         free(q);
      } else {
         p = p -> next;
      }
   }
}

int attempt6 ( char loc, int ref, int R1, int R2 )
{
   int r, min, rmin, score, i;
   place *p, *q;
   char name[16];

   min = nsymb + 1; rmin = 0;
   for (r=1; r<=nreg; ++r) {
      if ((r != R1) && (r != R2)) {
         p = R[r];
         score = 0;
         while (p) {
            if (p -> loc == 'M') i = p -> ref;
            else {
               sprintf(name, "$%d", p -> ref);
               i = STfind(name);
            }
            q = ST[i].storage;
            while (q) {
               if (q -> loc == 'M') break;
               q = q -> next;
            }
            if (q == NULL)  ++score;
            p = p -> next;
         }
         if (score < min) {
            min = score;
            rmin = r;
         }
      }
   }

   r = rmin;
   p = R[r];
   while (p) {
      if (p -> loc == 'M') i = p -> ref; 
      else {
         sprintf(name, "$%d", p -> ref);
         i = STfind(name);
      }
      q = ST[i].storage;
      while (q) {
         if (q -> loc == 'M') break;
         q = q -> next;
      }
      if (q == NULL) {
         storereg(p -> loc, p -> ref, r, i);
         delfromR(p -> loc, p -> ref, r);
         p = R[r];
      } else {
         p = p -> next;
      }
   }

   p = R[r];
   while (p) {
      q = p -> next;
      free(p);
      p = q;
   }
   R[r] = NULL;
   return r;
}

void setreg ( char loc, int ref, int r )
{
   place *p, *q;
   int i;
   char name[16];
   char LOC;
   int REF;

   p = (place *)malloc(sizeof(place));
   p -> loc = loc;
   p -> ref = ref;
   p -> next = R[r];
   R[r] = p;

   if (loc == 'T') {
      sprintf(name, "$%d", ref);
      i = STfind(name);
   } else {
      i = ref;
   }
   p = ST[i].storage;
   while (p) {
      if (p -> loc == 'R') {
         if (ST[i].name[0] == '$') {
            LOC = 'T';
            REF = atoi(ST[i].name + 1);
         } else {
            LOC = 'M';
            REF = i;
         }
         if ((LOC != loc) && (REF != ref))
            delfromR(LOC, REF, p -> ref);
      }
      q = p -> next;
      free(p);
      p = q;
   }
   ST[i].storage = (place *)malloc(sizeof(place));
   ST[i].storage -> loc = 'R';
   ST[i].storage -> ref = r;
   ST[i].storage -> next = NULL;
}

regs getreg ( quad Q, int inst, int end )
{
   regs IR;
   int lflag;

   IR.R = -1;

   if (Q.arg1loc == 'I') {
      IR.R1 = 0;
   } else {
      lflag = 1;
      IR.R1 = attempt1(Q.arg1loc, Q.arg1ref);
      if (IR.R1 != -1) lflag = 0;
      if (IR.R1 == -1) IR.R1 = attempt2(Q.arg1loc, Q.arg1ref);
      if (IR.R1 == -1) IR.R1 = attempt3(Q.arg1loc, Q.arg1ref);
      if (IR.R1 == -1)
         if ((Q.resloc == Q.arg1loc) && (Q.resref == Q.arg1ref))
            IR.R = IR.R1 = attempt4(Q.arg1loc, Q.arg1ref);
      if (IR.R1 == -1) IR.R1 = attempt5(Q.arg1loc, Q.arg1ref, inst, end);
      if (IR.R1 == -1) IR.R1 = attempt6(Q.arg1loc, Q.arg1ref, -1, -1);
      if (lflag) loadreg(LOD, Q.arg1loc, Q.arg1ref, IR.R1);
   }

   if (Q.arg2loc == 'I') {
      IR.R2 = 0;
   } else {
      lflag = 1;
      IR.R2 = attempt1(Q.arg2loc, Q.arg2ref);
      if (IR.R2 != -1) lflag = 0;
      if (IR.R2 == -1) IR.R2 = attempt2(Q.arg2loc, Q.arg2ref);
      if (IR.R2 == -1) IR.R2 = attempt3(Q.arg2loc, Q.arg2ref);
      if (IR.R2 == -1)
         if ((Q.resloc == Q.arg2loc) && (Q.resref == Q.arg2ref))
            IR.R = IR.R2 = attempt4(Q.arg2loc, Q.arg2ref);
      if (IR.R2 == -1) IR.R2 = attempt5(Q.arg2loc, Q.arg2ref, inst, end);
      if (IR.R2 == -1) IR.R2 = attempt6(Q.arg2loc, Q.arg2ref, IR.R1, -1);
      if (lflag) loadreg(LOD, Q.arg2loc, Q.arg2ref, IR.R2);
   }
   
   if (isop(Q.op)) {
      if (IR.R == -1) {
         IR.R = attempt1(Q.resloc, Q.resref);
         if (IR.R == -1) IR.R = attempt2(Q.resloc, Q.resref);
         if (IR.R == -1) IR.R = attempt3(Q.resloc, Q.resref);
         if (IR.R == -1) IR.R = attempt5(Q.resloc, Q.resref, inst, end);
         if (IR.R == -1) IR.R = attempt6(Q.resloc, Q.resref, IR.R1, IR.R2);
      }
      setreg(Q.resloc, Q.resref, IR.R);
   }

   return IR;
}

regs getreg2 ( quad Q, int inst, int end )
{
   regs IR;
   int lflag = 1;

   if (Q.arg1loc == 'I') {
      IR.R = attempt1(Q.resloc, Q.resref);
      if (IR.R1 != -1) lflag = 0;
      if (IR.R == -1) IR.R = attempt2(Q.resloc, Q.resref);
      if (IR.R == -1) IR.R = attempt3(Q.resloc, Q.resref);
      if (IR.R == -1) IR.R = attempt5(Q.resloc, Q.resref, inst, end);
      if (IR.R == -1) IR.R = attempt6(Q.resloc, Q.resref, -1, -1);
      loadreg(LDI, Q.arg1loc, Q.arg1ref, IR.R);
   } else {
      lflag = 1;
      IR.R1 = attempt1(Q.arg1loc, Q.arg1ref);
      if (IR.R1 != -1) lflag = 0;
      if (IR.R1 == -1) IR.R1 = attempt2(Q.arg1loc, Q.arg1ref);
      if (IR.R1 == -1) IR.R1 = attempt3(Q.arg1loc, Q.arg1ref);
      if (IR.R1 == -1)
         if ((Q.resloc == Q.arg1loc) && (Q.resref == Q.arg1ref))
            IR.R1 = attempt4(Q.arg1loc, Q.arg1ref);
      if (IR.R1 == -1) IR.R1 = attempt5(Q.arg1loc, Q.arg1ref, inst, end);
      if (IR.R1 == -1) IR.R1 = attempt6(Q.arg1loc, Q.arg1ref, -1, -1);
      if (lflag) loadreg(LOD, Q.arg1loc, Q.arg1ref, IR.R1);
      IR.R = IR.R1;
   }

   setreg(Q.resloc, Q.resref, IR.R);

   return IR;
}

void flushreg ( int start, int end )
{
   int i, loc, ref, r;
   place *p;

   for (i=start; i<=end; ++i) {
      if ((isop(QT[i].op)) || (QT[i].op == SET)) {
         loc = QT[i].resloc;
         ref = QT[i].resref;
         if (loc == 'M') {
            p = ST[ref].storage;
            while (p) {
               if (p -> loc == 'M') break;
               r = p -> ref;
               p = p -> next;
            }
            if (p == NULL) storereg(loc, ref, r, ref);
         }
      }
   }
}

void gencode ( int b )
{
   int start, end, i, j, flushed = 0;
   regs IR;

   leaderinst[b] = tnextinst;
   initreg();
   start = leader[b];
   end = leader[b+1] - 1;
   for (i=start; i<=end; ++i) {
      if (isop(QT[i].op)) {
         IR = getreg(QT[i],i,end);
         tcode[tnextinst].op = QT[i].op;
         if (IR.R1) {
            tcode[tnextinst].arg1loc = 'R';
            tcode[tnextinst].arg1ref = IR.R1;
         } else {
            tcode[tnextinst].arg1loc = 'I';
            tcode[tnextinst].arg1ref = QT[i].arg1ref;
         }
         if (IR.R2) {
            tcode[tnextinst].arg2loc = 'R';
            tcode[tnextinst].arg2ref = IR.R2;
         } else {
            tcode[tnextinst].arg2loc = 'I';
            tcode[tnextinst].arg2ref = QT[i].arg2ref;
         }
         tcode[tnextinst].resloc = 'R';
         tcode[tnextinst].resref = IR.R;
      } else if (QT[i].op == SET) {
         IR = getreg2(QT[i],i,end);
         --tnextinst;
      } else if (isrel(QT[i].op)) {
         IR = getreg(QT[i],i,end);
         flushreg(start, end); flushed = 1;
         switch (QT[i].op) {
            case EQ: tcode[tnextinst].op = NE; break;
            case NE: tcode[tnextinst].op = EQ; break;
            case LT: tcode[tnextinst].op = GE; break;
            case LE: tcode[tnextinst].op = GT; break;
            case GT: tcode[tnextinst].op = LE; break;
            case GE: tcode[tnextinst].op = LT; break;
            default: tcode[tnextinst].op = 0; break;
         }
         if (IR.R1) {
            tcode[tnextinst].arg1loc = 'R';
            tcode[tnextinst].arg1ref = IR.R1;
         } else {
            tcode[tnextinst].arg1loc = 'I';
            tcode[tnextinst].arg1ref = QT[i].arg1ref;
         }
         if (IR.R2) {
            tcode[tnextinst].arg2loc = 'R';
            tcode[tnextinst].arg2ref = IR.R2;
         } else {
            tcode[tnextinst].arg2loc = 'I';
            tcode[tnextinst].arg2ref = QT[i].arg2ref;
         }
         tcode[tnextinst].resref = -1;
         for (j=1; j<=nblock; ++j) {
            if (leader[j] == QT[i].resref)
               tcode[tnextinst].resref = j;
         }
      } else if (QT[i].op == JMP) {
         flushreg(start, end); flushed = 1;
         tcode[tnextinst].op = JMP;
         tcode[tnextinst].resref = -1;
         for (j=1; j<=nblock; ++j) {
            if (leader[j] == QT[i].resref)
               tcode[tnextinst].resref = j;
         }
      }
      ++tnextinst;
   }
   if (!flushed) flushreg(start, end);
}

void addjumpaddr ( )
{
   int i, j;

   for (i=1; i<tnextinst; ++i) {
      if (isrel(tcode[i].op) || (tcode[i].op == JMP)) {
         j = tcode[i].resref;
         tcode[i].resref = leaderinst[j];
      }
   }
}

void printvar ( char l, int r )
{
   if (l == 'R') printf(" R%d", r);
   else if (l == 'M') printf(" %s", ST[r].name);
   else if (l == 'T') printf(" $%d", r);
   else if (l == 'I') printf(" %d", r);
   else printf(" ??? (%d)", l);
}

void printopcode ( int op )
{
   switch (op) {
      case ADD: printf("ADD"); break;
      case SUB: printf("SUB"); break;
      case MUL: printf("MUL"); break;
      case DIV: printf("DIV"); break;
      case REM: printf("REM"); break;
      case EQ: printf("JEQ"); break;
      case NE: printf("JNE"); break;
      case LT: printf("JLT"); break;
      case LE: printf("JLE"); break;
      case GT: printf("JGT"); break;
      case GE: printf("JGE"); break;
      default: printf("???"); break;
   }
}

void prntcode ( )
{
   int i, j = 1;

   for (i=1; i<tnextinst; ++i) {
      if (i == leaderinst[j]) {
         printf("\nBlock %d\n", j);
         ++j;
      }
      printf("   %d\t: ", i);
      if (isop(tcode[i].op)) {
         printopcode(tcode[i].op);
         printvar(tcode[i].resloc, tcode[i].resref);
         printvar(tcode[i].arg1loc, tcode[i].arg1ref);
         printvar(tcode[i].arg2loc, tcode[i].arg2ref);
      } else if (isrel(tcode[i].op)) {
         printopcode(tcode[i].op);
         printvar(tcode[i].arg1loc, tcode[i].arg1ref);
         printvar(tcode[i].arg2loc, tcode[i].arg2ref);
         printf(" %d", tcode[i].resref);
      } else if (tcode[i].op == JMP) {
         printf("JMP");
         printf(" %d", tcode[i].resref);
      } else if (tcode[i].op == LOD) {
         printf("LD");
         printvar(tcode[i].resloc, tcode[i].resref);
         printvar(tcode[i].arg1loc, tcode[i].arg1ref);
      } else if (tcode[i].op == LDI) {
         printf("LDI");
         printvar(tcode[i].resloc, tcode[i].resref);
         printvar(tcode[i].arg1loc, tcode[i].arg1ref);
      } else if (tcode[i].op == STR) {
         printf("ST");
         printvar(tcode[i].resloc, tcode[i].resref);
         printvar(tcode[i].arg1loc, tcode[i].arg1ref);
      }
      printf("\n");
   }
   printf("\n   %d\t:\n", tnextinst);
}

int main ( int argc, char *argv[] )
{
   int i;

   if (argc > 1) nreg = atoi(argv[1]);
   initreg();

   yyparse();
   QT[nextinst].op = EXIT;
   printf("\n+++ Parsing successful\n");

   genleaders();
   printf("\n");
   for (i=0; i<40; ++i) printf("=");
   printf("\n+++ Blocks in three-address code\n");
   prnblocks();

   for (i=1; i<nblock; ++i) gencode(i);
   leaderinst[nblock] = tnextinst;
   addjumpaddr();

   printf("\n");
   for (i=0; i<40; ++i) printf("=");
   printf("\n+++ Target code\n");
   prntcode();

   exit(0);
}
