#include "assert.h"

void __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
  printf("__assert_func\n");
  return;
}