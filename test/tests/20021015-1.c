/* PR opt/7409.  */

extern void abort (void);

char g_list[1];// = { '1' };

void g (void *p, char *list, int length, char **elementPtr, char **nextPtr)
{
  if (*nextPtr != g_list)
    abort ();

  **nextPtr = 0;
}

int main (void)
{
  char *element;
  int i, length = 100;
  char *list; list = g_list;
  g_list[0] = '1';

  for (i = 0; *list != 0; i++) 
    {
      char *prevList; prevList = list;
      g (0, list, length, &element, &list);
      length -= (list - prevList);
    }

  return 0;
}

