main()
{
  if (1 % 3 != 1)
    exit(1);
  if (3 % 1 != 0)
    exit(2);
  if (5 % 3 != 2)
    exit(3);
  if (3 % 3 != 0)
    exit(4);
  if (-5 % 5 != 0)
    exit(5);
  if (-5 % 2 != -1)
    exit(6);
  if (5 % -2 != 1)
    exit(7);

  return 0;
}
