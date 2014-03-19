/*typedef*/ struct At
{
  short a;// __attribute__ ((aligned (2),packed));
  short *ap[2];//  __attribute__ ((aligned (2),packed));
};// A;

main ()
{
  short i, j = 1;
  struct At a, *ap; ap = &a;
  ap->ap[j] = &i;
  exit (0);
}
