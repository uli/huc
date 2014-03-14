int foo(int a)
{
  for (;;) {
    int b;
    b = a;
    return b;
  }
}

int main()
{
  int x;
  x = 42;
  foo(23);
  if (x != 42)
    abort();
  return 0;
}
