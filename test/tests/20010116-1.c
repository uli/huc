/* Distilled from optimization/863.  */

extern void abort (void);
extern void exit (int);
extern void ok (int);

struct Data
{
  int x, y, z;
};

void find (struct Data *first, struct Data *last)
{
  int i;
  for (i = (last - first) >> 2; i > 0; --i)
    ok(i);
  abort ();
}

void ok(int i)
{
  if (i != 1)
    abort ();
  exit (0);
}

int
main ()
{
  struct Data DataList[4];
  find (DataList + 0, DataList + 4);
}
