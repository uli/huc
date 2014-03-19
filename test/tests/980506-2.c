static void *self(void *p){ return p; }

struct x { int i;};

int
f()
{
  struct /*{ int i; }*/ x s, *sp;
  int *ip; ip = &s.i;

  s.i = 1;
  sp = self(&s);
  
  *ip = 0;
  return sp->i+1;
}

main()
{
  if (f () != 1)
    abort ();
  else
    exit (0);
}
