#include <stdio.h>
#include <string.h>
#include "builtins.h"

int
echo (char *message)
{
  message[strlen(message) - 1] = '\0';
  printf ("%s\n", message);
  return 0;
}

int
cd (char *message)
{
   FILE *fp = fopen (message, "r");
    if (fp == NULL)
    {
      perror(message);
      return -1;
    }
  
  return execvp (fp, '\0');
}

int
pwd (char *message)
{
  message[strlen(message) - 1] = '\0';
  printf ("%s\n", message);
  return 0;
}

int
type (char *message)
{
  if (*message == 'cd' || 
      *message == 'echo' ||
      *message == 'pwd' ||
      *message == 'type')
  {
    printf ("cmd is a shell built-in");
    return 0;
  }

  FILE *fp = fopen (message, "r");
    if (fp == NULL)
    {
      perror(message);
      return -1;
    }

}
