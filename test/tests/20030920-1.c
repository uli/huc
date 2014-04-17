extern void abort (void);

const  char *str = "\x7f\xff";
int main()
{
  int hicount = 0;
  unsigned char *c;
  for (c = (unsigned char *)str; *c ; c++) {
    if (!(((unsigned int)(*c)) < 0x80)) hicount++;
  }
  if (hicount != 1)
    abort ();
  return 0;
}

