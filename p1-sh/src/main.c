#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cmd.h"
#include "hash.h"
#include "shell.h"

static bool get_args (int, char **, FILE **);
static void usage (void);

int
main (int argc, char *argv[])
{
  FILE *script = stdin;
  if (!get_args (argc, argv, NULL))
    return EXIT_FAILURE;
  return EXIT_SUCCESS;
}

/* Parse the command-line arguments. See the usage documentation below for the
   required interface. */
static bool
get_args (int argc, char **argv, FILE **script)
{
  int ch = 0;
  while ((ch = getopt (argc, argv, "h")) != -1)
    {
      switch (ch)
        {
        case 'b':
          *script = fopen (optarg, "r");
          if (*script == NULL)
          {
            perror (optarg);
            return false;
          }
          break;
        case 'f':
          break;
        case 'h':
          usage ();
          return false;
        default:
          return false;
        }
    }
  return true;
}

static void
usage (void)
{
  printf ("dukesh, a simple command shell\n");
  printf ("usage: dukesh [-b FILE]\n");
  printf ("  -b FILE    use FILE as a shell script to execute\n");
  printf ("  -f         show FSM interpretation of command line processing\n");
  printf ("If no script is passed, then the shell should be interactive,\n");
  printf ("processing one command at a time from STDIN.\n");
  printf ("If the -f flag is passed, the command lines from the input will\n");
  printf ("be parsed as a sequence of FSM transitions, but the line will\n");
  printf ("not actually be executed.\n");
}
