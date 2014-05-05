const int b = 4;
const int *a = &b;
const int d = b+1;
const int e = 1+b;

int main()
{
	if (d != 5)
		exit(1);
	if (*a != 4)
		exit(2);
	if (e != 5)
		exit(3);
	return 0;
}
