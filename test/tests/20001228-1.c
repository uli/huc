union ut {
    char a[2];
    unsigned int b;
};
int foo1(void)
{
  union ut u;
  u.b = 0x01;
  return u.a[0];
}

  union ut2 {
    char a[/* sizeof (unsigned) */ 2];
    unsigned b;
  };

int foo2(void)
{
  /* volatile */ union ut2 u;
  u.b = 0x01;
  return u.a[0];
}

int main(void)
{
  if (foo1() != foo2())
    abort ();
  exit (0);
}
