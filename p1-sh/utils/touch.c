#include <stdio.h>
#include <stdlib.h>

static void usage (void);

int
main (int argc, char *argv[])
{
  return EXIT_SUCCESS;
}

static void __attribute__ ((unused))
usage (void)
{
  printf ("touch, create an empty file or update its timestamp\n");
  printf ("usage: touch FILE...\n");
}
