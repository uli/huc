struct x {
	int a, b;
};
struct x z;

int foo(struct x *p, int y)
{
  if ((y & 0xff) != y || -p->b >= p->a)
    return 1;
  return 0;
}

main()
{
  z.a = -4028;
  z.b = 4096;
  if (foo (&z, 10))
    abort ();
  exit (0);
}
