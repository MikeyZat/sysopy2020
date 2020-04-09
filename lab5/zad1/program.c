#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  int value, add = 0;
  if (argc == 2)
  {
    add = (int)strtol(argv[1], NULL, 10);
  }

  while (scanf("%d", &value) > 0)
    printf("%d\n", value + add);
}