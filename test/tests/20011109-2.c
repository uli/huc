int main(void)
{
  char *c1, *c2;
  int i;
  c1 = "foo";
  c2 = "foo";
  for (i = 0; i < 3; i++)
    if (c1[i] != c2[i])
      abort ();
  exit (0);
}
