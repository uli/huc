unsigned char **foo;
const unsigned char *bar[2] = { "ABC", "DEF" };

int main()
{
	foo = bar;
	if (foo[1][1] != 'E')
		abort();
	return 0;
}

