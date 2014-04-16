typedef int foo_t;

int main()
{
	foo_t x = 0;
	if (sizeof(x) != 2)
		abort();
	return x;
}
