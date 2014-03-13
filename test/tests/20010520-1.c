static const unsigned int expr_hash_table_size = 1;

int
main ()
{
  int del;
  unsigned int i;
  i = 0;
  del = 1;

  if (i < expr_hash_table_size && del)
    exit (0);
  abort ();
}
