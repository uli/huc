const char *foo[2] = { "ABC", "DEF" };

int main()
{
	if (foo[1][1] != 'E')
		abort();
	return 0;
}
