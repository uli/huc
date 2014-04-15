#ifdef VISUAL
#define GOOD(x) put_string("Good", 0, x);
#define BAD(x) put_string("BAD", 5, x);
#else
#define GOOD(x)
#define BAD(x) exit(x);
#endif

int main() {
#ifdef VISUAL
	load_default_font();
#endif
#if 0
	BAD(1)
#elif 1
	GOOD(1)
#endif

#if 1
	GOOD(2)
#elif 1
	BAD(2)
#endif

#if 0
	BAD(3)
#elif 0
	BAD(3)
#else
	GOOD(3)
#endif

#if 1
	GOOD(4)
#elif 0
	BAD(4)
#endif

#if 0
	BAD(5)
#elif 0
	BAD(5)
#elif 1
	GOOD(5)
#else
	BAD(5)
#endif

#if 0
	BAD(6)
#elif 0
	BAD(6)
#elif 1
	GOOD(6)
#elif 1
	BAD(6)
#else
	BAD(6)
#endif

#if 0
	BAD(7)
#elif 0
	BAD(7)
#elif 1
	GOOD(7)
#elif 0
	BAD(7)
#else
	BAD(7)
#endif

#if 0
#if 0
	BAD(8)
#elif 0
	BAD(8)
#elif 1
	BAD(8)
#else
	BAD(8)
#endif
#else
	GOOD(8)
#endif

#if 1
#if 0
	BAD(9)
#elif 0
	BAD(9)
#elif 1
	GOOD(9)
#else
	BAD(9)
#endif
#else
	BAD(9)
#endif

#if 0
#if 0
	BAD(10)
#elif 1
	BAD(10)
#endif
#else
#if 1
	GOOD(10)
#else
	BAD(10)
#endif
#endif

#if 1
#if 0
	BAD(11)
#elif 1
	GOOD(11)
#endif
#else
#if 1
	BAD(11)
#else
	BAD(11)
#endif
#endif

#if 0
	BAD(12)
#elif 1
#if 0
	BAD(12)
#elif 0
	BAD(12)
#else
	GOOD(12)
#endif
#elif 1
	BAD(12)
#endif

	return 0;
}
