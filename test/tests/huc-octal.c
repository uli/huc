f()
{
  return ("\177"[0]);
}

main()
{
  if (f() != (0177))
    abort();
  exit (0);
}
