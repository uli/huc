const int loop_1 = 100;
const int loop_2 = 7;
int flag;

int test (void)
{
    int i;
    int counter;
    counter = 0;

    while (loop_1 > counter) {
        if (flag & 1) {
            for (i = 0; i < loop_2; i++) {
                counter++;
            }
        }
        flag++;
    }
    return 1;
}

int main()
{
    flag = 0;
    if (test () != 1)
      abort ();
    
    exit (0);
}

