int a,b;

struct foo {
	int a;
	int b;
};

int main()
{
	struct foo x;
	x.a = 5;
	x.b = 4;
	a = 3;
	b = 2;
	if (x.a != 5)
		abort();
	if (a != 3)
		abort();
	if (b != 2)
		abort();
	if (x.b != 4)
		abort();
	return 0;
}
