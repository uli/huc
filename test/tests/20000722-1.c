struct s { char *p; int t; };

extern void bar (void);
extern void foo (struct s *);

int main(void)
{
  bar ();
  bar ();
  exit (0);
}

void 
bar (void)
{
  struct s ss;
  ss.p = "hi";
  ss.t = 1;
  foo (&ss);
}

void foo (struct s *p)
{
  if (p->t != 1)
    abort();
  p->t = 2;
}
