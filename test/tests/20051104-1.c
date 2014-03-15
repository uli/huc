/* PR rtl-optimization/23567 */

struct st
{
  int len;
  char *name;
}; struct st s;

int
main (void)
{
  s.len = 0;
  s.name = "";
  if (s.name [s.len] != 0)
    s.name [s.len] = 0;
  return 0;
}
