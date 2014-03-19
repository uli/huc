struct null {
};

main()
{
  if(sizeof(struct null) != 0)
    abort();
  return 0;
}

