struct foo {
  int b;
  char *a;
};

const char *bla = "Blabla";
struct foo f[2];
struct foo *fp;

main()
{
  char **x;
  struct foo **y;
  x = &bla;
  fp = f;
  f[0].a = bla;
  if (*x != bla)
    exit(1);

  /* Failed to dereference this correctly. */
  if (**x != 'B')
    exit(2);

  /* This stuff never failed, but better to check it anyway. */
  y = &fp;
  if (*y != fp)
    exit(3);
  if ((*y)->a != bla)
    exit(4);
  if (*(*y)->a != 'B')
    exit(5);
  if ((*y)->a[1] != 'l')
    exit(6);

  return 0;
}
