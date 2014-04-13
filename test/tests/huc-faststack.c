#asm
_foo.2:
  cla
  cpx #27
  bne .x
  clx
.x:
  rts
#endasm

#pragma fastcall foo(word ax, byte)

main()
{
  unsigned char i,j;
  int f;
  f = 9;
  i = 42;
  j = 23;
  return foo(0, 4+j);
}
