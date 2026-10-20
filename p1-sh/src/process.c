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
#include <fcntl.h>

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
  if (waitpid (pid, &status, 0) == -1)
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

int
run_args (char **argv)
{
  if (argv == NULL || argv[0] == NULL)
    return 0;

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
  if (waitpid (pid, &status, 0) == -1)
    {
      perror ("waitpid");
      return -1;
    }

  if (WIFEXITED (status))
    return WEXITSTATUS (status);

  return -1;
}

int
run_args_redirect (char **argv, const char *stdout_file)
{
  if (argv == NULL || argv[0] == NULL)
    return 0;

  fflush (stdout);
  pid_t pid = fork ();
  if (pid < 0)
    {
      perror ("fork");
      return -1;
    }

  if (pid == 0)
    {
      if (stdout_file != NULL)
        {
          int fd = open (stdout_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
          if (fd == -1)
            {
              perror (stdout_file);
              _exit (1);
            }

          if (fd != STDOUT_FILENO)
            {
              if (dup2 (fd, STDOUT_FILENO) == -1)
                {
                  perror ("dup2");
                  close (fd);
                  _exit (1);
                }
              close (fd);
            }
        }
      execvp (argv[0], argv);
      perror (argv[0]);
      _exit (127);
    }

  int status = 0;
  if (waitpid (pid, &status, 0) == -1)
    {
      perror ("waitpid");
      return -1;
    }

  if (WIFEXITED (status))
    return WEXITSTATUS (status);

  return -1;
}

int
run_pipe (char **left, char **right)
{
  int pipefd[2];

  if (pipe (pipefd) == -1)
    {
      perror ("pipe");
      return -1;
    }

  fflush (stdout);

  pid_t producer = fork ();
  if (producer < 0)
    {
      perror ("fork");
      close (pipefd[0]);
      close (pipefd[1]);
      return -1;
    }

  if (producer == 0)
    {
      if (dup2 (pipefd[1], STDOUT_FILENO) == -1)
        {
          perror ("dup2");
          _exit (1);
        }

      close (pipefd[0]);
      close (pipefd[1]);
      execvp (left[0], left);
      perror (left[0]);
      _exit (127);
    }

  pid_t consumer = fork ();
  if (consumer < 0)
    {
      perror ("fork");
      close (pipefd[0]);
      close (pipefd[1]);
      waitpid (producer, NULL, 0);
      return -1;
    }

  if (consumer == 0)
    {
      if (dup2 (pipefd[0], STDIN_FILENO) == -1)
        {
          perror ("dup2");
          _exit (1);
        }

      close (pipefd[0]);
      close (pipefd[1]);

      execvp (right[0], right);
      perror (right[0]);
      _exit (127);
    }

    close (pipefd[0]);
    close (pipefd[1]);

    int producer_status = 0;
    int consumer_status = 0;

    pid_t producer_result = waitpid (producer, &producer_status, 0);
    pid_t consumer_result = waitpid (consumer, &consumer_status, 0);

    if (producer_result == -1 || consumer_result == -1)
      {
        perror ("waitpid");
        return -1;
      }

    if (WIFEXITED (consumer_status))
      return WEXITSTATUS (consumer_status);

    return -1;
}
