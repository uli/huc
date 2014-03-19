int glob;

g (int x)
{
  glob = x;
  return 0;
}

f (int x)
{
  int a; a = ~x;
  while (a)
    a = g (a);
}

main ()
{
  f (3);
  if (glob != -4)
    abort ();
  exit (0);
}
