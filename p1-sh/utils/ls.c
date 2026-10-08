#include <stdio.h>
#include <stdlib.h>

static void usage (void);

// WARNING WARNING WARNING:
// When using opendir and readdir to read directory listings, the files
// are returned in the order they were created. There is randomness to that
// creation that can cause tests to fail. You MUST sort the file names as
// expected in the test case files. To avoid making your task harder than
// it needs to be, we strongly suggest you use scandir instead.

int
main (int argc, char *argv[])
{
  return EXIT_SUCCESS;
}

static void __attribute__ ((unused))
usage (void)
{
  printf ("ls, list directory contents\n");
  printf ("usage: ls [FLAG ...] [DIR]\n");
  printf ("FLAG is one or more of:\n");
  printf ("  -a       list all files (even hidden ones)\n");
  printf ("  -p       list permission bitmask\n");
  printf ("  -s       list file sizes\n");
  printf ("If no DIR specified, list current directory contents.\n\n");
  printf ("Files must be sorted alphabetically, case insensitive.\n");
  printf ("Leading dots should be ignored when sorting.\n\n");
  printf ("With the -s flag, do not show entries for subdirectories.\n");
  printf ("Permission bitmasks are 10-character strings such as:\n");
  printf ("  -rwxr-x---\n\n");
  printf ("The first character is d for directories and - for regular files.\n\n");
  printf ("Do not show the \".\" or \"..\" directory entries.\n");
}
