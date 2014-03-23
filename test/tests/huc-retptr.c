struct s {
  int a;
  int b;
};

struct s arr[2];

struct s *foo(void)
{
  return &arr[0];
}

int main()
{
  struct s* sp;
  sp = foo() + 1;
  if (sp != &arr[1])
    abort();
  return 0;
}
