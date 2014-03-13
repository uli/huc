/* PR c/5430 */
/* Verify that the multiplicative folding code is not fooled
   by the mix between signed variables and unsigned constants. */

extern void abort (void);
extern void exit (int);

int main (void)
{
  int my_int;// = 924;
  unsigned int result;
  my_int = 924;

  result = ((my_int*2 + 4) - 8/*U*/) / 2;
  if (result != 922/*U*/)
    abort();
         
  result = ((my_int*2 - 4/*U*/) + 2) / 2;
  if (result != 923/*U*/)
    abort();

  result = (((my_int + 2) * 2) - 8/*U*/ - 4) / 2;
  if (result != 920/*U*/)
    abort();
  result = (((my_int + 2) * 2) - (8/*U*/ + 4)) / 2;
  if (result != 920/*U*/)
    abort();

  result = ((my_int*4 + 2/*U*/) - 4/*U*/) / 2;
  if (result != 1847/*U*/)
    abort();

  exit(0);
}
