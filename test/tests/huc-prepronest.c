#define A(a, b) (a) + (b)
#define B(a, b) (a) * (b)

int main()
{
	if (A(1, B(2, 3)) != 7)
		abort();
	return 0;
}
