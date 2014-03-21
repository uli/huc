struct eins {
  char a;
};
struct zwei {
  int a;
};
struct drei {
  int a;
  char b;
};
struct vier {
  int a;
  int b;
};
struct fuenf {
  int a;
  int b;
  char c;
};
struct sechs {
  int a;
  int b;
  int c;
};
struct sieben {
  int a;
  int b;
  int c;
  char d;
};
struct acht {
  int a;
  int b;
  int c;
  int d;
};
struct neun {
  int a;
  int b;
  int c;
  int d;
  char e;
};

struct eins one[3];
struct zwei two[3];
struct drei three[3];
struct vier four[3];
struct fuenf five[3];
struct sechs six[3];
struct sieben seven[3];
struct acht eight[3];
struct neun nine[3];

int main()
{
  int i;
  for (i = 0; i < 3; i++) {
    one[i].a = 41;
    two[i].a = 42;
    three[i].b = 43;
    four[i].b = 44;
    five[i].c = 45;
    six[i].c = 46;
    seven[i].d = 47;
    eight[i].d = 48;
    nine[i].e = 49;
  }
  for (i = 0; i < 3; i++) {
    if (one[i].a != 41)
      exit(1);
    if (two[i].a != 42)
      exit(2);
    if (three[i].a != 0 || three[i].b != 43)
      exit(3);
    if (four[i].a != 0 || four[i].b != 44)
      exit(4);
    if (five[i].a != 0 || five[i].b != 0 || five[i].c != 45)
      exit(5);
    if (six[i].a != 0 || six[i].b != 0 || six[i].c != 46)
      exit(6);
    if (seven[i].a != 0 || seven[i].b != 0 || seven[i].c != 0 || seven[i].d != 47)
      exit(7);
    if (eight[i].a != 0 || eight[i].b != 0 || eight[i].c != 0 || eight[i].d != 48)
      exit(8);
    if (nine[i].a != 0 || nine[i].b != 0 || nine[i].c != 0 || nine[i].d != 0 || nine[i].e != 49)
      exit(9);
  }
  return 0;
}
