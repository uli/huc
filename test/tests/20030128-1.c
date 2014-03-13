unsigned char x;// = 50;
/* volatile */ const short y = -5;

int main ()
{
  x = 50;
  x /= y;
  if (x != /*(unsigned char) -10 */ 0xf6)
    abort ();
  exit (0);
}
