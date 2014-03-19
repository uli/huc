char *
f (char *p)
{
  short x; x = *p++ << 16;
  return p;
}

main ()
{
  char *p; p = "";
  if (f (p) != p + 1)
    abort ();
  exit (0);
}
