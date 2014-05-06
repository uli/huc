struct foo {
	int a;
	int b;
};

int x = 10;
char y = 11;
int z[3] = {3, 2, 1};
struct foo zz = {2, 1};
struct foo zzz[2] = {{7,6},{5,4}};
struct foo zzzz[3] = {{23, 24}, {25, 26}, {27, 28}};
struct foo aaa[2];
int main()
{
	static int a = 5;
	static char b = 3;
	static int c[3] = {1, 2, 3};
	static struct foo d = {1, 2};
	static struct foo e[2] = {{4, 5}, {6, 7}};

	if (e[0].a != 4 || e[0].b != 5 || e[1].b != 7 || e[1].a != 6)
		exit(1);
	if (d.b != 2 || d.a != 1)
		exit(2);
	if (c[2] != 3 || c[1] != 2 || c[0] != 1)
		exit(3);
	if (b != 3 || a != 5)
		exit(4);
	if (aaa[0].a || aaa[0].b || aaa[1].a || aaa[1].b)
		exit(5);
	if (zzzz[2].b != 28 || zzzz[2].a != 27 ||
	    zzzz[1].b != 26 || zzzz[1].a != 25 ||
	    zzzz[0].b != 24 || zzzz[0].a != 23)
		exit(6);
	if (zzz[1].b != 4 || zzz[1].a != 5 ||
	    zzz[0].b != 6 || zzz[0].a != 7)
		exit(7);
	if (zz.b != 1 || zz.a != 2)
		exit(8);
	if (z[2] != 1 || z[1] != 2 || z[0] != 3)
		exit(9);
	if (y != 11 || x != 10)
		exit(10);
	return 0;
}
