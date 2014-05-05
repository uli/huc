struct foo {
	unsigned char a;
	unsigned char b;
};
struct foo a = {1, 2};
int b = 1000;

int main()
{
	if (a.b != 2)
		exit(1);
	if (a.a != 1)
		exit(2);
	if (b != 1000)
		exit(3);
	return 0;
}
