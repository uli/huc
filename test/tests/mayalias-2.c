struct S { short x; };
//typedef struct S __attribute__((__may_alias__)) test;
#define test struct S

int f() {
  int a=10;
  test *p; p =/*(test *)*/&a;
  p->x = 1;
  return a;
}

int main() {
  if (f() == 10)
    /*__builtin_*/abort();
  return 0;
}


