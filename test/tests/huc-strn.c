#include <string.h>

const char *x = "Icks";
const char *y = "Uepsilon";

int main()
{
  char a[20];
  a[0] = 0;
  strncat(a, x, 6);
  if (strcmp(a, x))
    exit(1);
  strncat(a, x, 6);
  if (strcmp(a, "IcksIcks"))
    exit(2);
  a[0] = 0;
  if (strncat(a, x, 2) != a)
    exit(3);
  if (strcmp(a, "Ic"))
    exit(4);
  if (strncpy(a, y, 5000) != a)
    exit(5);
  if (strcmp(a, y))
    exit(6);
  strncpy(a, x, 2);
  if (strcmp(a, "Icpsilon"))
    exit(7);
  exit(0);
}
