f()
{
  unsigned b = 0;

  if (b > ~0/*U*/)
    b = ~0/*U*/;

  return b;
}
main()
{
  if (f()!=0)
    abort();
  exit (0);
}
