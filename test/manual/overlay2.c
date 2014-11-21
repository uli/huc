#include <huc.h>

#ifdef INIT_GLOBAL2
int a = 2;
#else
int a;
#endif

extern char __huc_rodata;
extern char __huc_rodata_end;
extern char __huc_data;
int main()
{
  load_default_font();
  put_string("Lo", 0, 2);
  put_hex(&__huc_rodata, 4, 0, 3);
  put_hex(&__huc_rodata_end, 4, 5, 3);
  put_hex(&__huc_data, 4, 10, 3);
  put_number(a, 5, 15, 3);
  for(;;);
}
