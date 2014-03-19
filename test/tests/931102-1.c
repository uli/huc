struct x {
  char h,l;
};
union T
{
  /*struct
    {
      char h, l;
    } b; */
  struct x b;
};

f (x)
     int x;
{
  int num = 0;
  union T reg;

  reg.b.l = x;
  while ((reg.b.l & 1) == 0)
    {
      num++;
      reg.b.l >>= 1;
    }
  return num;
}

main ()
{
  if (f (2) != 1)
    abort ();
  exit (0);
}

