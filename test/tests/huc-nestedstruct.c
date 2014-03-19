struct a {
  int a, b;
};
struct b {
  int c;
  struct a d;
};

main()
{
  struct b x;
  x.c = 1;
  x.d.a = 2;
  x.d.b = 3;
  if (x.c != 1)
    abort();
  x.c = 4;
  if (x.d.a != 2)
    abort();
  if (x.d.b != 3)
    abort();
  return 0;
}
