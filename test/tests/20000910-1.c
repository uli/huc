/* Copyright (C) 2000  Free Software Foundation  */
/* by Alexandre Oliva <aoliva@redhat.com> */

/* #include <stdlib.h> */

void bar (int);
void foo (int *);

const int a[] = { 0, 1, 2 };
int main () {
  int *i;
  i = &a[sizeof(a)/sizeof(/* *a */ int)];
  
  while (i-- > a)
    foo (i);

  exit (0);
}

void baz (int, int);

void bar (int i) { baz (i, i); }
void foo (int *i) { bar (*i); }

void baz (int i, int j) {
  if (i != j)
    abort ();
}
