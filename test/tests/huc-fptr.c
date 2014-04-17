void foo(void)
{
}

int bar;
int main()
{
	bar = 0;
	if (!foo)
		abort();
	return 0;
}
