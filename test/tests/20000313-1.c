unsigned int buggy (unsigned int *param)
{
  unsigned int accu, zero, borrow;
  zero = 0;
  accu    = - *param;
  borrow  = - (accu > zero);
  *param += accu;
  return borrow;
}

int main (void)
{
  unsigned int param;
  unsigned int borrow;
  param = 1;
  borrow = buggy (&param);

  if (param != 0)
    abort ();
  if (borrow + 1 != 0)
    abort ();
  return 0;
}
