char *f() {
  return "f";
}
char *g() {
  return "g";
}

main()
{
  if (!strcmp(f(), g()))
    abort();
  if (strcmp(f(), f()))
    abort();
  if (strcmp(f(), "f"))
    abort();
  if (!strcmp(g(), "f"))
    abort();
  if (strcmp("f", f()))
    abort();
  if (!strcmp("g", f()))
    abort();
  return 0;
}
