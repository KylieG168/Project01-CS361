#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// You may assume that lines are no longer than 1024 bytes
#define LINELEN 1024

static void usage (void);

int
main (int argc, char *argv[])
{
  const char *input_name = NULL;
  int option;

  while ((option = getopt (argc, argv, "f:")) != -1)
    {
      if (option != 'f')
        {
          usage ();
          return EXIT_FAILURE;
        }
      input_name = optarg;
    }
  if (argc - optind > 1)
    return EXIT_FAILURE;

  FILE *input = stdin;
  if (input_name != NULL)
    {
      input = fopen (input_name, "r");
      if (input == NULL)
        return EXIT_FAILURE;
    }

  FILE *output = NULL;
  if (optind < argc)
    {
      output = fopen (argv[optind], "w");
      if (output == NULL)
      {
        if (input != stdin)
          fclose (input);

        return EXIT_FAILURE;
      }
    }

  int result = EXIT_SUCCESS;
  char buffer[LINELEN + 1];

  while (fgets (buffer, sizeof (buffer), input) != NULL)
    {
      if (fputs (buffer, stdout) == EOF)
        {
          result = EXIT_FAILURE;
          break;
        }

      if (output != NULL && fputs (buffer, output) == EOF)
        {
          result = EXIT_FAILURE;
          break;
        }
    }

  if (ferror (input))
    result = EXIT_FAILURE;

  if (input != stdin && fclose (input) == EOF)
    result = EXIT_FAILURE;

  if (output != NULL && fclose (output) == EOF)
    result = EXIT_FAILURE;

  return result;
}

static void __attribute__ ((unused))
usage (void)
{
  fprintf (stderr, "tee, reads a file stream and writes it to a file\n");
  fprintf (stderr, "usage: tee [-f IN] [FILE]\n");
  fprintf (stderr, "FLAG can be:\n");
  fprintf (stderr, "  -f IN    use IN as input instead of STDIN\n");
  fprintf (stderr, "If no FILE specified, write to STDOUT\n");
}
