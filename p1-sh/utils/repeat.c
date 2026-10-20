#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static void usage (void);

int
main (int argc, char *argv[])
{
  if (argc < 3 || (argc - 1) % 2 != 0)
    {
      usage ();
      return EXIT_FAILURE;
    }

  for (int i = 1; i < argc; i += 2)
    {
      char *end = NULL;
      errno = 0;
      long count = strtol (argv[i], &end, 10);

      if (errno != 0 || end == argv[i] || *end != '\0' || count <= 0)
        {
          usage ();
          return EXIT_FAILURE;
        }

      const char *name = argv[i + 1];
      const char *value = getenv (name);

      if (value == NULL)
        value = "";

      for (long j = 0; j < count; j++)
        printf ("%s=%s\n", name, value);
    }

  return EXIT_SUCCESS;
}

static void __attribute__ ((unused))
usage (void)
{
  printf ("repeat, a tool for printing repeated environment variables\n");
  printf ("usage: repeat N VAR ...\n");
  printf ("each N must be a positive integer\n");
  printf ("N VAR can be repeated, but each repetition must have both\n");
}
