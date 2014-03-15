char **p;
const char *a = "Huhu";
const char *b = "Hallo";
char *ptr[2];

int main()
{
  ptr[0] = a;
  ptr[1] = b;
  if (strcmp(ptr[1], "Hallo"))
    abort();
  if (strcmp(ptr[0], "Huhu"))
    abort();
  p = &ptr[0];
  if (strcmp(*p, "Huhu"))
    abort();
  p++;
  if (strcmp(*p, "Hallo"))
    abort();
  return 0;
}
