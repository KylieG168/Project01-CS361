#include <stdio.h>
#include <stdlib.h>

// You may assume that lines are no longer than 1024 bytes
#define LINELEN 1024

static void usage (void);

int
main (int argc, char *argv[])
{
  return EXIT_SUCCESS;
}

static void __attribute__ ((unused))
usage (void)
{
  fprintf (stderr, "tee, reads a file stream and writes it to a file\n");
  fprintf (stderr, "usage: head [FLAG] FILE\n");
  fprintf (stderr, "FLAG can be:\n");
  fprintf (stderr, "  -f IN    use IN as input instead of STDIN\n");
  fprintf (stderr, "If no FILE specified, write to STDOUT\n");
}
