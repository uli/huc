#pragma fastcall foo(farptr _fbank:_fptr)

#asm
  .data
  .bank 9
_data:	.ds 1
  .code

_foo.1:
  lda	__fbank
  cmp	#9
  beq	.next
.abort:
  call	_abort
.next:
  lda	__fptr
  cmp	#low(_data)
  bne	.abort
  lda	__fptr+1
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
