// Implementation of functions related to executing a command as a
// separate process. In later stages, you should add IPC and I/O
// redirection functionality here.

// You should provide some function like this to serve as the interface to
// parsing the command line. This function is just a placeholder and you
// should define your own.
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "hash.h"
#include "process.h"

void
free_environment (char **environment)
{
  if (environment == NULL)
    return;

  for (size_t i = 0; environment[i] != NULL; i++)
    free (environment[i]);

  free (environment);
}

char **
build_environment (void)
{
  char **keys = hash_keys ();
  if (keys == NULL)
    return NULL;

  size_t count = 0;
  while (keys[count] != NULL)
    count++;

  char **environment = calloc (count + 1, sizeof (char *));
  if (environment == NULL)
    {
      free (keys);
      return NULL;
    }

  size_t used = 0;

  for (size_t i = 0; i < count; i++)
    {
      if (strcmp (keys[i], "?") == 0)
        continue;

      const char *value = hash_find (keys[i]);
      if (value == NULL)
        continue;

      size_t length = strlen (keys[i]) + strlen (value) + 2;
      environment[used] = malloc (length);

      if (environment[used] == NULL)
        {
          free (keys);
          free_environment (environment);
          return NULL;
        }

      snprintf (environment[used], length, "%s=%s", keys[i], value);
      used++;
    }

  free (keys);
  return environment;
}

static void
exec_with_environment (char **argv)
{
  char **environment = build_environment ();
  if (environment == NULL)
    {
      fprintf (stderr, "Could not build environment\n");
      _exit (1);
    }

  int error = ENOENT;

  if (strchr (argv[0], '/') != NULL)
    {
      execve (argv[0], argv, environment);
      error = errno;
    }
  else
    {
      const char *path = hash_find ("PATH");
      if (path == NULL)
        path = "/usr/bin:/bin";

      const char *directory = path;

      while (1)
        {
          const char *colon = strchr (directory, ':');
          size_t length = colon != NULL ? (size_t)(colon - directory)
                                        : strlen (directory);
          size_t size = length + strlen (argv[0]) + 2;
          char *candidate = malloc (size);

          if (candidate == NULL)
            {
              error = ENOMEM;
              break;
            }

          if (length == 0)
            {
              snprintf (candidate, size, "%s", argv[0]);
            }
          else
            {
              memcpy (candidate, directory, length);
              candidate[length] = '/';
              strcpy (candidate + length + 1, argv[0]);
            }

          execve (candidate, argv, environment);
          int attempt_error = errno;
          free (candidate);

          if (attempt_error == EACCES)
            error = EACCES;
          else if (attempt_error != ENOENT && attempt_error != ENOTDIR)
            {
              error = attempt_error;
              break;
            }

          if (colon == NULL)
            break;

          directory = colon + 1;
        }
    }

  free_environment (environment);
  errno = error;
  perror (argv[0]);
  _exit (error == ENOENT || error == ENOTDIR ? 127 : 126);
}

int
run_process (char *cmd)
{
  if (cmd == NULL)
    return 0;

  char *argv[101] = { NULL };
  int argc = 0;
  char *save = NULL;

  for (char *tok = strtok_r (cmd, " \t\r\n", &save); tok != NULL && argc < 100;
       tok = strtok_r (NULL, " \t\r\n", &save))
    {
      argv[argc++] = tok;
    }

  argv[argc] = NULL;
  return run_args (argv);
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
    exec_with_environment (argv);

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
      exec_with_environment (argv);
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
      exec_with_environment (left);
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
      exec_with_environment (right);
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
