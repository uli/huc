char a, b, c;

int main()
{
  a = 5; b = 3;
#asm
  lda _a
  clc
  adc _b
  sta _c
#endasm
  if (c != 8)
    abort();
  return 0;
}
