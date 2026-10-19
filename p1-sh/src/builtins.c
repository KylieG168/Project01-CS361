#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE 100

int
echo (char *message)
{
  size_t len = strnlen (message, MAX_LINE);
  if (len > 0 && message[len - 1] == '\n')
    message[len - 1] = '\0';

  for (int i = 0; message[i] != '\0'; i++)
    {
      if (message[i] == '\\' && message[i + 1] == 'n')
        {
          printf ("\n");
          i++;
        }
      else
        {
          printf ("%c", message[i]);
        }
    }

  printf ("\n");
  return 0;
}

int
cd (char *message)
{
  while (*message == ' ' || *message == '\t')
    {
      message++;
    }

  size_t len = strnlen (message, MAX_LINE);
  if (len > 0 && message[len - 1] == '\n')
    message[len - 1] = '\0';

  if (message[0] == '\0')
    {
      return 0;
    }
  if (chdir (message) != 0)
    {
      perror (message);
      return -1;
    }
  return 0;
}

int
pwd (char *message)
{
  (void)message;
  char cwd[4096];
  if (getcwd (cwd, sizeof cwd) == NULL)
    {
      perror ("pwd");
      return 1;
    }
  printf ("%s\n", cwd);
  return 0;
}

int
type (char *message)
{
  size_t len = strnlen (message, MAX_LINE);
  if (len > 0 && message[len - 1] == '\n')
    {
      message[len - 1] = '\0';
    }
  if (message[0] == '\0')
    {
      return 0;
    }

  const char *names[]
      = { "cd", "echo", "pwd", "quit", "type" }; // all the types
  for (size_t i = 0; i < sizeof names / sizeof names[0]; i++)
    {
      if (strcmp (message, names[i]) == 0) // if it is a builtin
        {
          printf ("%s is a shell builtin\n", message);
          return 0;
        }
    }

  const char *path = getenv ("PATH"); // if it is not a builtin
  if (path != NULL)
    {
      char *copy = strdup (path);
      if (copy == NULL)
        {
          perror ("strdup");
          return 1;
        }

      char *save = NULL;
      for (char *dir = strtok_r (copy, ":", &save); dir != NULL; // split at :
           dir = strtok_r (NULL, ":", &save))
        {
          char full[4096];
          snprintf (full, sizeof full, "%s/%s", dir, message);
          if (access (full, X_OK) == 0) // checks if it's possible to execute
            {
              printf ("%s is %s\n", message,
                      full); // prints if it is executable
              free (copy);
              return 0;
            }
        }
      free (copy); // frees the copy if it is not executable
    }
  printf ("%s not found\n", message);
  return 1;
}
