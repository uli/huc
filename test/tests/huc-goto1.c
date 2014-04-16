int main()
{
	goto foo;
	abort();
foo:
	return 0;
}
