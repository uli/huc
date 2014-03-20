/* PR tree-optimization/33017 */
/* { dg-do run } */
/* { dg-options "-O2 -ftree-vectorize" } */

#define __SIZE_TYPE__ unsigned int

extern __SIZE_TYPE__ strlen (const char *);
extern void abort (void);

char *
//__attribute__((noinline))
foo (const char *string)
{
  int len;
  static char var[0x104];
  int off;
  len = strlen (string);
  for (off = 0; off < 64; off++)
    var[len + off + 2] = 0x57;
  return var;
}

int
main (void)
{
  int i;
  char *p;p = foo ("abcd");
  for (i = 0; i < 0x104; i++)
    if (p[i] != ((i >= 6 && i < 70) ? 0x57 : 0))
      abort ();
  return 0;
}
