extern void foo(void);
void bar(void);

int main()
{
	if (foo)
		return 0;
	if (bar)
		return 0;
	abort();
}

void foo(void)
{
}

#asm
	.proc _bar
	.endp
#endasm
