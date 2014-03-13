extern void exit (int);
extern void abort (void);

struct xt { int a; int b[5]; }; struct xt x;
int *y;

int foo (void)
{
  return y - x.b;
}

int main (void)
{
  y = x.b;
  if (foo ())
    abort ();
  exit (0);
}
