int main()
{
	unsigned char a,b;
	a = 1;
	b = 1;
	a = b = 0;
	if (a || b)
		abort();
	return 0;
}
