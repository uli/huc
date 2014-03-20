/* PR optimization/6703
   Origin: Glen Nakamura <glen@imodulo.com> */
/* { dg-do run } */
/* { dg-options "-O2" } */

extern void abort (void);
extern void exit (int);

void foo (int *x, int y)
{
  /*__builtin_*/memset (x, 0, y);
}
  
int main ()
{
  int x[2]; x[0]=-1;x[1]=-1;// = { -1, -1 };
    
  if (x[1] != -1)
    abort ();
  foo (x, sizeof (int) + 1);
  if (x[1] == -1)
    abort ();
  exit (0);
}
