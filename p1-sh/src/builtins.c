#include "builtins.h"
#include "hash.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int
echo (char *message)
{
  // message[strlen(message) - 1] = '\0';
  // printf ("%s\n", message);
  // return 0;
  char *save = NULL;
  char *word = strtok_r (message, " \t\r\n", &save);
  char delimiter = ' ';

  if (word != NULL && strncmp (word, "-d", 2) == 0 && strlen (word) == 3)
    {
      delimiter = word[2];
      word = strtok_r (NULL, " \t\r\n", &save);
    }

  int first = 1;
  while (word != NULL)
    {
      if (!first)
        putchar (delimiter);

      if (strcmp (word, "$?") == 0)
        {
          char *status = hash_find ("?");
          printf ("%s", status != NULL ? status : "0");
        }
      else
        {
          printf ("%s", word);
        }

      first = 0;
      word = strtok_r (NULL, " \t\r\n", &save);
    }
  putchar ('\n');
  return 0;
}
