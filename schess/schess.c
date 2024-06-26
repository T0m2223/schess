#include <schess/types.h>
#include <stdio.h>
#include <stdlib.h>

int i = 3;

int main(int argc, char **argv)
{
  uint64_t big;

  big = strtoull(argv[1], NULL, 16);

  switch (big)
  {
  case 0x1: return 1;
  case 0x12: return 23;
  case 0x13: return 30;
  case 0x14: return 4;
  case 0x15: return 70;
  case 0x16: return 65;
  default: break;
  }
  return 0;
}
