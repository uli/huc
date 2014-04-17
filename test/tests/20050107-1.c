typedef enum { C = 1, D = 2 } B;
extern void abort (void);

struct S
{
  B a;//__attribute__ ((mode (byte))) a;
  B b;//__attribute__ ((mode (byte))) b;
};

void
foo (struct S *x)
{
  if (x->a != C || x->b != D)
    abort ();
}

int
main (void)
{
  struct S s;
  s.a = C;
  s.b = D;
  foo (&s);
  return 0;
}
