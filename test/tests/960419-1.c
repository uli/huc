static int i;

void
check(x)
     int x;
{
  if (!x)
    abort();
}

main()
{
  int *p; p = &i;

  check(p != /*(void *)*/0);
  exit (0);
}
