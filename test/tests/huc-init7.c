struct foo {
	int a;
	int b;
};

int main()
{
	static struct foo a = {1, 2};
	if (a.b != 2 || a.a != 1)
		abort();
	return 0;
}
