unsigned
sat_add (unsigned i)
{
  unsigned ret; ret = i + 1;
  if (ret < i)
    ret = i;
  return ret;
}

main ()
{
  if (sat_add (~0/*U*/) != ~0/*U*/)
    abort ();
  exit (0);
}
