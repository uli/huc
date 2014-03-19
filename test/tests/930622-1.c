int a/* = 1*/, b;

g () { return 0; }
h (int x) {}

f ()
{
  if (g () == -1)
    return 0;
  a = g ();
  if (b >= 1)
    h (a);
  return 0;
}

main ()
{
  a = 1;
  f ();
  if (a != 0)
    abort ();
  exit (0);
}
