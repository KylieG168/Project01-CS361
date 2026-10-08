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
  printf ("chmod, changes permissions on a file\n");
  printf ("usage: chmod USR GRP OTH FILE\n\n");
  printf ("USR, GRP, and OTH must be of the rwx format,\n");
  printf ("with - indicating a permission is not allwed.\n");
}
