#include <stdio.h>
#include <stdlib.h>

static void usage (void);

int
main (int argc, char *argv[])
{
  if (argc != 2)
    {
      usage ();
      return EXIT_FAILURE;
    }

  FILE *fp = fopen (argv[1], "r");
  if (fp == NULL)
    {
      perror (argv[1]);
      return EXIT_FAILURE;
    }

  char buf[4096];
  size_t n;
  while ((n = fread (buf, 1, sizeof buf, fp)) > 0)
    {
      if (fwrite (buf, 1, n, stdout) != n)
        {
          fclose (fp);
          return EXIT_FAILURE;
        }
    }

  int err = ferror (fp);
  fclose (fp);
  if (err != 0)
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

static void __attribute__ ((unused))
usage (void)
{
  printf ("cat, print the contents of a file\n");
  printf ("usage: cat FILE\n");
}
