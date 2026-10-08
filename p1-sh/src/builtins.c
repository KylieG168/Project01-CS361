#include <stdio.h>

#include "builtins.h"

int
echo (char *message)
{
  printf ("%s\n", message);
  return 0;
}
