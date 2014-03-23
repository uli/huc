int foo()
{
  return 5000;
}

int main()
{
  /* "+ 3" was incorrectly promoted to byte size */
  if (1 + foo() + 3 != 5004)
    abort();
  return 0;
}
