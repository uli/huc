const short x1 = 17;

struct tt
{
  short i;// __attribute__ ((packed));
}; struct tt t;

f ()
{
  t.i = x1;
  if (t.i != 17)
    abort ();
}

main ()
{
  f ();
  exit (0);
}
