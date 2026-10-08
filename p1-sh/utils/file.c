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
  printf ("file, get a summary of information about a file\n");
  printf ("usage: file [-t] FILE...\n");
  printf ("  -t       include language type for source code files\n");
}
