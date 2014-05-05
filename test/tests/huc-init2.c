int a = 3;
int b[3] = {4, 5, 6};
int c = 7;

int main()
{
	if (b[0] != 4 || b[1] != 5 || b[2] != 6)
		abort();
	return a != 3 || c != 7;
}
