#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

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
      int fd = open (argv[i], O_WRONLY | O_CREAT, 0666);
      if (fd < 0)
        {
          perror (argv[i]);
          rc = EXIT_FAILURE;
          continue;
        }
      if (futimens (fd, NULL) != 0)       /* NULL means "now" */
        {
          perror (argv[i]);
          rc = EXIT_FAILURE;
        }
      close (fd);
    }
  return rc;
}

static void __attribute__ ((unused))
usage (void)
{
  printf ("touch, create an empty file or update its timestamp\n");
  printf ("usage: touch FILE...\n");
}
