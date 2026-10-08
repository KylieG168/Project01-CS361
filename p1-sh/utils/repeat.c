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
  printf ("repeat, a tool for printing repeated environment variables\n");
  printf ("usage: repeat N VAR ...\n");
  printf ("each N must be a positive integer\n");
  printf ("N VAR can be repeated, but each repetition must have both\n");
}
