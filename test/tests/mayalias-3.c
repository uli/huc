struct S { short x; };
//typedef struct S __attribute__((__may_alias__)) test;
#define test struct S

test *p;

int g(int *a)
{
 p = /*(test*)*/a;
}

int f()
{
  int a;
  test s;
  g(&a);
  a = 10;
  //test s={1};
  s.x = 1;
  *p=s;
  return a;
}

int main() {
  if (f() == 10)
    /*__builtin_*/abort();
  return 0;
}


