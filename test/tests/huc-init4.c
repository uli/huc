int x = 10;
struct foo {
	int a;
	int b;
} bar = {4, 5};
int y = 12;

int main()
{
	if (bar.b != 5)
		exit(1);
	if (bar.a != 4)
		exit(2);
	if (y-x != 2)
		exit(3);
	return 0;
}
