#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

// You may assume that lines are no longer than 1024 bytes
#define LINELEN 1024

static void usage (void);

int
main (int argc, char *argv[])
{
  long line_limit = 5;
  int option;
  while ((option = getopt (argc, argv, "n:")) != -1)
    {
      if (option != 'n')
        {
          usage ();
          return EXIT_FAILURE;
        }

      char *end = NULL;
      errno = 0;
      long value = strtol (optarg, &end, 10);

      if (errno != 0 || end == optarg || *end != '\0' || value < 0)
        {
          return EXIT_FAILURE;
        }
      line_limit = value;
    }

  FILE *input = stdin;

  if (optind < argc)
    {
      input = fopen (argv[optind], "r");
      if (input == NULL)
        {
          return EXIT_FAILURE;
        }
    }

  char buffer[LINELEN + 1];
  long lines_read = 0;

  while (lines_read < line_limit
         && fgets (buffer, sizeof (buffer), input) != NULL)
    {
      fputs (buffer, stdout);
      lines_read++;
    }
  int result = ferror (input) ? EXIT_FAILURE : EXIT_SUCCESS;

  if (input != stdin)
    {
      if (fclose (input) == EOF)
        result = EXIT_FAILURE;
    }
  return result;
}

static void __attribute__ ((unused))
usage (void)
{
  printf ("head, prints the first few lines of a file\n");
  printf ("usage: head [FLAG] FILE\n");
  printf ("FLAG can be:\n");
  printf ("  -n N     show the first N lines (default 5)\n");
  printf ("If no FILE specified, read from STDIN\n");
}
