const char a[5] = {1, 2, 3, 4, 5};

main()
{
  unsigned char u = 2;
  char s = -1;
  int i = 1;
  char *b;
  b = a + 2;
  if (b[u] != 5)
    exit(1);
  if (b[s] != 2)
    exit(2);
  if (b[i] != 4)
    exit(3);
  return 0;
}
