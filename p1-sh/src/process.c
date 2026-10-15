// Implementation of functions related to executing a command as a
// separate process. In later stages, you should add IPC and I/O
// redirection functionality here.

// You should provide some function like this to serve as the interface to
// parsing the command line. This function is just a placeholder and you
// should define your own.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "process.h"

int
run_process (char *cmd)
{
  if (cmd == NULL)
  {
    return 0;
  }

  char *argv[101] = { NULL };
  int argc = 0;
  char *save = NULL;

  char *token = strtok_r (cmd, " \t\r\n", &save);
  while (token != NULL)
    {
      argv[argc] = token;
      argc++;
      token = strtok_r (NULL, " \t\r\n", &save);
    }

  argv[argc] = NULL;

  if (argc == 0)
    {
      return 0;
    }

  // argv[0] = strtok_r (cmd, " \t", &save);
  // if (argv[0] == NULL)
  // {
  //   return 0;
  // }
  // argv[1] = strtok_r (NULL, " \t", &save);

  fflush (stdout);

  pid_t pid = fork ();
  if (pid < 0)
    {
      perror ("fork");
      return -1;
    }

  if (pid == 0)
    {
      execvp (argv[0], argv);
      perror (argv[0]);
      _exit (127);
    }

  int status = 0;
  if ( waitpid (pid, &status, 0) == -1)
    {
      perror ("waitpid");
      return -1;
    }

  if (WIFEXITED (status))
    {
      return WEXITSTATUS (status);
    }
  return -1;
}
/*
  int
  ls ()
  {

  }

  int
  head ()
  {
    if (arg[1] == NULL)
    {

    }
  }
*/
