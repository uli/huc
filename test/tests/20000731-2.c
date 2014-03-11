int
main()
{
    int i, j;
    i = 1;
    j = 0;

    while (i != 1024 || j <= 0) {
        i *= 2;
        ++ j;
    }

    if (j != 10)
      abort ();

    exit (0);
}
