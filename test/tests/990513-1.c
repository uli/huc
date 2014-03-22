#include <string.h>

#define SIZ STACK_SIZE / 4

void foo (int *BM_tab, int j)
{
  int *BM_tab_base;

  BM_tab_base = BM_tab;
  BM_tab += SIZ;
  while (BM_tab_base != BM_tab)
    {
      *--BM_tab = j;
      *--BM_tab = j;
      *--BM_tab = j;
      *--BM_tab = j;
    }
}

int main ()
{
  int BM_tab[SIZ];
  memset (BM_tab, 0, sizeof (BM_tab));
  foo (BM_tab, 6);
  if (BM_tab[0] != 6)
    abort ();
  return 0;
}
