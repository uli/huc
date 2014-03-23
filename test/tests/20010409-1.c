//typedef __SIZE_TYPE__ size_t;
#define size_t unsigned int

extern size_t strlen (const char *s);

/*typedef*/ struct A_s {
  int a, b;
};// A;
#define A struct A_s

/*typedef*/ struct B_s {
  struct A_s **a;
  int b;
};// B;
#define B struct B_s

A *a;
int b /*= 1*/, c;
B d[1];

void foo (A *x, const char *y, int z)
{
  c = y[4] + z * 25;
}

A *bar (const char *v, int w, int x, const char *y, int z)
{
  if (w)
    abort ();
  exit (0);
}

void test (const char *x, int *y)
{
  foo (d->a[d->b], "test", 200);
  d->a[d->b] = bar (x, b ? 0 : 65536, strlen (x), "test", 201);
  d->a[d->b]->a++;
  if (y)
    d->a[d->b]->b = *y;
}

int main ()
{
  b = 1;
  d->b = 0;
  d->a = &a;
  test ("", 0);
}
