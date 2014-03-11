unsigned short int showbug(unsigned short int *a, unsigned short int *b)
{
        *a += *b -8;
        return (*a >= 8);
}

int main()
{
        unsigned short int x;
        unsigned short int y;
        x = 0;
        y = 10;

        if (showbug(&x, &y) != 0)
	  abort ();

	exit (0);
}

