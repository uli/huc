const short int i = -1;
const char * /*const*/ wordlist[207];

const char * /*const*/ *
foo(void)
{
  register /*const*/ char * /*const*/ *wordptr; wordptr = &wordlist[207/*u*/ + i];
  return wordptr;
}

int
main()
{
  if (foo() != &wordlist[206])
    abort ();
  exit(0);
}
