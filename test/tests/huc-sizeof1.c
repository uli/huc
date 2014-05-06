struct foo {
	int a;
	int b;
};

int x = 10;
char y = 11;
int z[3] = {3, 2, 1};
struct foo zz = {2, 1};
struct foo zzz[2] = {{7,6},{5,4}};
struct foo aaa[2];
int main()
{
	static int a = 5;
	static char b = 3;
	static int c[3] = {1, 2, 3};
	static struct foo d = {1, 2};
	static struct foo e[2] = {{4, 5}, {6, 7}};
	struct foo aa[2];
	
	if (sizeof(a) <= 1 || sizeof(x) <= 1)
		exit(1);
	if (sizeof(b) != 1 || sizeof(y) != 1)
		exit(2);
	if (sizeof(c) != sizeof(a) * 3)
		exit(3);
	if (sizeof(z) != sizeof(a) * 3)
		exit(4);
	if (sizeof(d) != sizeof(a) * 2)
		exit(5);
	if (sizeof(zz) != sizeof(a) * 2)
		exit(6);
	if (sizeof(e) != sizeof(a) * 4)
		exit(7);
	if (sizeof(zzz) != sizeof(a) * 4)
		exit(8);
	if (sizeof(aa) != sizeof(a) * 4)
		exit(9);
	if (sizeof(aaa) != sizeof(a) * 4)
		exit(10);
	return 0;
}
