/* inline */ int fff (int x)
    {
      return x*10;
    }
int main ()
{
  int v;
  v = 42;


  return (fff (v) != 420);
}
