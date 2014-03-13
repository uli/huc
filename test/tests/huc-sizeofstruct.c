struct X {
  int a;
  int b[3];
  char c;
};
  
main()
{
  if (sizeof(struct X) != sizeof(int) * 4 + sizeof(char))
    abort();
  return 0;
}
