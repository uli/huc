/* same as huc-farptr.c, but with a prototype instead of a pragma */
void __fastcall foo(int far *data<fbank:fptr>);

#asm
  .data
  .bank 9
_data:	.ds 1
  .code

_foo.1:
  lda	fbank
  cmp	#9
  beq	.next
.abort:
  call	_abort
.next:
  lda	fptr
  cmp	#low(_data)
  bne	.abort
  lda	fptr+1
  cmp	#high(_data)
  bne	.abort
  rts
#endasm
extern int data[];

int main()
{
  foo(data);
  return 0;
}
