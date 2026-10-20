#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

static void usage (void);

int
main (int argc, char *argv[])
{
  if (argc < 2)
    {
      usage ();
      return EXIT_FAILURE;
    }

  int rc = EXIT_SUCCESS;
  for (int i = 1; i < argc; i++)
    {
      if (mkdir (argv[i], 0777) != 0)   /* the umask trims these bits */
        {
          perror (argv[i]);
          rc = EXIT_FAILURE;
        }
    }
  return rc;
}

static void __attribute__ ((unused))
usage (void)
{
  printf ("mkdir, create a directory\n");
  printf ("usage: mkdir DIR...\n");
}
