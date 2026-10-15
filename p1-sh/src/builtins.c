#include <stdio.h>
#include <string.h>
#include "builtins.h"
#include <unistd.h>
int
echo (char *message)
{
  message[strlen(message) - 1] = '\0';
  printf ("%s\n", message);
  return 0;
}

