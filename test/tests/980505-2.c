//typedef unsigned short Uint16;
//typedef unsigned int Uint;
#define Uint16 unsigned short
#define Uint unsigned int

Uint f ()
{
        Uint16 token;
        Uint count;
        static Uint16 values[1]; values[0] = 0x9300;

        token = values[0];
        count = token >> 8;

        return count;
}

int
main ()
{
  if (f () != 0x93)
    abort ();
  exit (0);
}
