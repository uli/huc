/* PR target/21297 */
//typedef __SIZE_TYPE__ size_t;
#define size_t unsigned int
extern int memcmp (const char *, const char *, size_t);
extern void abort ();

void
foo (char *x)
{
  int i;
  for (i = 0; i < 2; i++);
  x[i + i] = '\0';
}

void
bar (char *x)
{
  int i;
  for (i = 0; i < 2; i++);
  x[i + i + i + i] = '\0';
}

const char *x_ro = "IJKLMNOPQR";

int
main (void)
{
  //char x[] = "IJKLMNOPQR";
  char x[11];
  strcpy(x, x_ro);
  foo (x);
  if (memcmp (x, "IJKL\0NOPQR", /*sizeof x*/11) != 0)
    abort ();
  x[4] = 'M';
  bar (x);
  if (memcmp (x, "IJKLMNOP\0R", /*sizeof x*/11) != 0)
    abort ();
  return 0;
}
