const char foo[] = {1, 2, 3, 4, 5};
int main()
{
  load_vram(0, foo, 5);
  vram[3] = 6;
  if (vram[0] != 0x0201 || vram[1] != 0x0403)
    abort();
  if (vram[3] != 6)
    abort();
  return 0;
}
