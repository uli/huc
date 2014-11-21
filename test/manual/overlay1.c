#include <huc.h>

#ifdef INIT_GLOBAL1
int a = 1;
#else
int a;
#endif

extern char __huc_rodata;
extern char __huc_rodata_end;
extern char __huc_data;
int main()
{
  load_default_font();
  put_string("Hi", 0, 0);
  put_hex(&__huc_rodata, 4, 0, 1);
  put_hex(&__huc_rodata_end, 4, 5, 1);
  put_hex(&__huc_data, 4, 10, 1);
  put_number(a, 5, 15, 1);
  
  cd_execoverlay(2);
  for(;;);
}
