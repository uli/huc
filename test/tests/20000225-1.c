int main ()
{
    int nResult;
    int b;
    int i;
    b = 0;
    i = -1;

    do
    {
     if (b!=0) {
       abort ();
       nResult=1;
     } else {
      nResult=0;
     }
     i++;
     b=(i+2)*4;
    } while (i < 0);
    exit (0);
}


