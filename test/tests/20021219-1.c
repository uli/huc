/* PR optimization/8988 */
/* Contributed by Kevin Easton */

void foo(char *p1, /* char ** */ void *p2)
{}
 
int main(void)
{
  char *str, *ptr; str = "foo { xx }";
  ptr = str + 5;

  foo(ptr, &ptr);

  while (*ptr && (*ptr == 13 || *ptr == 32))
    ptr++;

  return 0;
}
