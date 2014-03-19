const int a[5] = {1, 2, 3, 4, 5};
int *ap[3];

main()
{
  int *b, *c;
  int **d;
  b = a;
  c = &a[3];
  d = &b;
  if (*d != a)
    abort();
  if (**d != 1)
    abort();
  (*d)++;
  if (**d != 2)
    abort();
  ap[0] = a;
  ap[1] = b;
  ap[2] = 0;
  d = &ap[0];
  if (*d != a)
    abort();
  d++;
  if (*d++ != b)
    abort();
  return 0;
}
