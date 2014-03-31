const char a[5] = {4, 3, 2, 1, 0};
unsigned char i;

main()
{
  char *x;
  x = a + 2;
  i = 0;
  if (x[i] != 2)
    exit(1);
  i = 1;
  if (x[i] != 1)
    exit(2);
  return 0;
}
