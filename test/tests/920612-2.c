  int i;// = 0;
  int a (int x)
    {
      while (x)
	i++, x--;
      return x;
    }
main ()
{
  i = 0;
#ifndef NO_TRAMPOLINES
  if (a (2) != 0)
    abort ();
#endif
  exit (0);
}
