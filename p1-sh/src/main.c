#include "cmd.h"
#include "hash.h"
#include "shell.h"
#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static bool get_args (int, char **, FILE **);
static void usage (void);
bool fsm_debug = false;

int
main (int argc, char *argv[])
{
  FILE *in = stdin;
  if (!get_args (argc, argv, &in))
    return EXIT_FAILURE;

  shell (in);
  if (in != stdin)
    {
      if (fclose (in) == EOF)
        return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

/* Parse the command-line arguments. See the usage documentation below for the
   required interface. */
static bool
get_args (int argc, char **argv, FILE **script)
{
  int ch = 0;
  while ((ch = getopt (argc, argv, "b:fh")) != -1)
    {
      switch (ch)
        {
        case 'f':
          fsm_debug = true;
          break;
        case 'b':
          {
            FILE *fp = fopen (optarg, "r");
            if (fp == NULL)
              {
                perror (optarg);
                return false;
              }
            else
              {
                *script = fp;
                return true;
              }
            break;
          }
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
