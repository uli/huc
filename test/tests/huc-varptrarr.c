int a, b, c, d;

int *p[4];

main()
{
  p[0] = &a; p[1] = &b; p[2] = &c; p[3] = &d;
  a = 1; b = 2; c = 3; d = 4;
  if (*p[0] != 1 || *p[1] != 2 || *p[2] != 3 || *p[3] != 4)
    abort();
  return 0;
}
