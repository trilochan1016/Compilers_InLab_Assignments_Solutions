int pwr ( int a, int e )
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

void eprn ( int A[], int idx )
{
   printf("+++ Standalone expression evaluates to %d\n", (A) ? A[idx] : idx);
}

void mprn ( int A[], int idx )
{
   printf("+++ MEM[%d] set to %d\n", idx, A[idx]);
}
