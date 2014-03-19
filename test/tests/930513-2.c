sub3 (i)
     /*const*/ int *i;
{
}

eq (int a, int b)
{
  static int i;// = 0;
  if (a != i)
    abort ();
  i++;
}

main ()
{
  int i;

  for (i = 0; i < 4; i++)
    {
      /*const*/ int j, k; j = i;
      //int k;
      sub3 (&j);
      k = j;
      eq (k, k);
    }
  exit (0);
}
