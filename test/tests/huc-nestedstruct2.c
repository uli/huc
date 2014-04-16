struct foo {
	int a;
};
struct bar {
	struct foo b;
};
struct baz {
	struct bar c;
};
int main()
{
	struct baz x;
	x.c.b.a = 0;
	return x.c.b.a;
}
