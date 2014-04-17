typedef enum foo x;
enum bar {
  VIER = 4,
  FUENF,
  SECHS
};
enum foo {
  EINS = 1,
  ZWEI,
  DREI
};
int main()
{
	x a = ZWEI;
	return a - 2;
}
