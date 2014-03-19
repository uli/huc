/* PR c/19606
   The C front end used to shorten the type of a division to a type
   that does not preserve the semantics of the original computation.
   Make sure that won't happen.  */

signed char a;// = -4;

int
foo (void)
{
  return (/*(unsigned int) (signed int)*/ a) / 2/*LL*/;
}

int
bar (void)
{
  return (/*(unsigned int) (signed int)*/ a) % 5/*LL*/;
}

int
main (void)
{
  int r;
  a = -4;

  r = foo ();
  if (r != (/*(unsigned int) (signed int)*/ /*(signed char)*/ -4) / 2/*LL*/)
    abort ();

  r = bar ();
  if (r != (/*(unsigned int) (signed int)*/ /*(signed char)*/ -4) % 5/*LL*/)
    abort ();

  exit (0);
}
