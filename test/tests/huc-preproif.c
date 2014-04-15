int main()
{
	int a;
#if 0
	exit(1);
#endif
#define FOO 1
#if !FOO
	exit(2);
#endif
#ifdef NIL
#ifdef FOO
	exit(42);
#endif
	exit(43);
#endif

#if 0
#if FOO
	exit(3);
#endif
	exit(4);
#endif

	a = 1;
#if 1 + FOO
	a = 0;
#endif
	if (a)
		exit(5);
#if 0
	exit(6);
#else
#endif

#define BAR
#ifndef BAR
	exit(7);
#endif
#undef BAR
#ifdef BAR
	exit(8);
#endif
#ifdef FOO
	return 0;
#endif
	return 42;
}
