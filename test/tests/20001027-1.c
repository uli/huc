int x;
int *p;

int main()
{
  int i;
  p = &x;
  i=0;
  x=1;
  p[i]=2;
  if (x != 2)
    abort ();
  exit (0);
}
