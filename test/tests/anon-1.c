/* Copyright (C) 2001 Free Software Foundation, Inc.  */

/* Source: Neil Booth, 4 Nov 2001, derived from PR 2820 - field lookup in
   nested anonymous entities was broken.  */

struct foot
{
  int x;
  struct yyt
  {
    int a;
    union zzt
    {
      int b;
    }; union zzt zz;
  }; struct yyt yy;
}; struct foot foo;

int
main(int argc, char *argv[])
{
  foo.yy.zz.b = 6;
  foo.yy.a = 5;

  if (foo.yy.zz.b != 6)
    abort ();

  return 0;
}
