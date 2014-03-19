main()
{
  int i;
  if (__builtin_ffs(0x8000) != 16)
    exit(1);
  if (__builtin_ffs(0xa5a5) != 1)
    exit(2);
  if (__builtin_ffs(0x5a5a) != 2)
    exit(3);
  if (__builtin_ffs(0x0ca0) != 6)
    exit(4);
  if (__builtin_ffs(0x0c80) != 8)
    exit(5);
  if (__builtin_ffs(0x0d00) != 9)
    exit(6);
  for (i = 0; i < 16; i++) {
    if (__builtin_ffs(1 << i) != i+1)
      abort();
  }
  return 0;
}
