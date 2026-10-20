#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void usage (void);

static int
parse_perms (const char *s, mode_t r, mode_t w, mode_t x, mode_t *out)
{
  if (strnlen (s, 4) != 3)
    return 0;
  if (s[0] != 'r' && s[0] != '-')
    return 0;
  if (s[1] != 'w' && s[1] != '-')
    return 0;
  if (s[2] != 'x' && s[2] != '-')
    return 0;

  *out = 0;
  if (s[0] == 'r')
    *out |= r;
  if (s[1] == 'w')
    *out |= w;
  if (s[2] == 'x')
    *out |= x;
  return 1;
}

int
main (int argc, char *argv[])
{
  if (argc != 5)
    {
      usage ();
      return EXIT_FAILURE;
    }

  mode_t user, group, other;
  if (!parse_perms (argv[1], S_IRUSR, S_IWUSR, S_IXUSR, &user)
      || !parse_perms (argv[2], S_IRGRP, S_IWGRP, S_IXGRP, &group)
      || !parse_perms (argv[3], S_IROTH, S_IWOTH, S_IXOTH, &other))
    {
      usage ();
      return EXIT_FAILURE;
    }

  if (chmod (argv[4], user | group | other) != 0)
    {
      perror (argv[4]);
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

static void __attribute__ ((unused))
usage (void)
{
  printf ("chmod, changes permissions on a file\n");
  printf ("usage: chmod USR GRP OTH FILE\n\n");
  printf ("USR, GRP, and OTH must be of the rwx format,\n");
  printf ("with - indicating a permission is not allowed.\n");
}
