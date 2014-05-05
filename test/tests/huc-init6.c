struct foo {
	int a;
	int b;
};
struct foo bar[2] = {{1, 2}, {3,4}};

int main()
{
	if (bar[1].a != 3)
		exit(1);
	if (bar[0].b != 2)
		exit(2);
	if (bar[1].b != 4)
		exit(3);
	if (bar[0].a != 1)
		exit(4);
	return 0;
}
