int a[3] = {1,1,1};
int b[3] = {2};
int c[3] = {3,3,3};

int main()
{
	if (a[0] != 1 || a[1] != 1 || a[2] != 1)
		exit(1);
	if (c[0] != 3 || c[1] != 3 || c[2] != 3)
		exit(2);
	if (b[0] != 2 || b[1] || b[2])
		exit(3);
	return 0;
}
