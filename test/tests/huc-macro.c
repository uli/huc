#define NADA return
#undef NADA
#define BAZ return

#define BLA(x, y) if (y)
#define FOO(x, y) int x = y
#define BAR(x) (x+x+x)

int main()
{
  FOO(a, 1);
  if (1 != a)
  	exit(1);
  a += BAR(4);
  if (a != 13)
  	exit(2);
  BLA(5, 0)
  	exit(3);
  BAZ 0;
}
