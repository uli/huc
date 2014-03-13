extern void abort (void);

int
foo (int x)
{
  if (/*(int)*/ (x & 0x80ff) != /*(int)*/ (0x800e))
    abort ();

  return 0;
}

int
main ()
{
  return foo (0x800e);
}
