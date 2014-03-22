main()
{
  int a = -256;
  if (a >> 8 != -1)
    abort();
  return 0;
}
