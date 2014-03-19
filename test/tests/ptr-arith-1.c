char *
f (char *s, unsigned int i)
{
  return &s[i + 3 - 1];
}

main ()
{
  char *str, *x2; str = "abcdefghijkl";
  /*char **/x2 = f (str, 12);
  if (str + 14 != x2)
    abort ();
  exit (0);
}
