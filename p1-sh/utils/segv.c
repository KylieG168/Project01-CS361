#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *null = NULL;

static void usage (void);

int
main (void)
{
  printf ("null = %s\n", null);
  fflush (stdout);
  *null = 'a';
  printf ("null = %s\n", null);

  return EXIT_SUCCESS;
}

static void __attribute__ ((unused))
usage (void)
{
  printf ("segv, a segmentation fault generator\n");
  printf ("usage: segv\n");
  printf ("Add a signal handler to print \"ERROR: segfault %%d\n\" with the");
  printf ("received signal number to stderr instead of the typical message\n");
}
