main()
{
  int a = 1 + 42;
  int b = 2*23;
  int c = 2 * 23 + 1;
  int d = 1 + 2 * 23;
  int e = -5;
  int f = -5+3;
  int g = 3+-5;
  int h = ~0;

  if (a != 43) exit(1);
  if (b != 46) exit(2);
  if (c != 47) exit(3);
  if (d != 47) exit(4);
  if (e != -5) exit(5);
  if (f != -2) exit(6);
  if (g != -2) exit(7);
  if (h != -1) exit(8);
  return 0;
}
