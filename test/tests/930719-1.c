int
f (foo, bar, com)
int foo, bar, com;
{
  unsigned align;
  if (foo)
    return 0;
  while (1)
    {
      switch (bar)
	{
	case 1:
	  if (com != 0)
 	    return align;
	  // Excuse me? *(char *) 0 = 0;
	  abort();
	}
    }
}

main ()
{
  f (0, 1, 1);
  exit (0);
}
