#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void usage (void);

static int
remove_tree (const char *path)
{
  struct stat st;
  if (lstat (path, &st) != 0)
    {
      perror (path);
      return 1;
    }

  if (!S_ISDIR (st.st_mode))
    {
      if (unlink (path) != 0)
        {
          perror (path);
          return 1;
        }
      return 0;
    }

  DIR *dir = opendir (path);
  if (dir == NULL)
    {
      perror (path);
      return 1;
    }

  int rc = 0;
  struct dirent *entry;
  while ((entry = readdir (dir)) != NULL)
    {
      if (strcmp (entry->d_name, ".") == 0
          || strcmp (entry->d_name, "..") == 0)
        continue;

      char child[PATH_MAX];
      int n = snprintf (child, sizeof child, "%s/%s", path, entry->d_name);
      if (n < 0 || (size_t) n >= sizeof child)
        {
          rc = 1;
          continue;
        }
      if (remove_tree (child) != 0)
        rc = 1;
    }
  closedir (dir);

  if (rmdir (path) != 0)
    {
      perror (path);
      rc = 1;
    }
  return rc;
}

int
main (int argc, char *argv[])
{
  if (argc < 2)
    {
      usage ();
      return EXIT_FAILURE;
    }
  int first = 1;
  int recursive = 0;

  if (argc > 1 && strcmp (argv[1], "-r") == 0)
    {
      recursive = 1;
      first = 2;
    }
  if (first >= argc)
    {
      usage ();
      return EXIT_FAILURE;
    }

  int rc = EXIT_SUCCESS;
  for (int i = first; i < argc; i++)
    {
      if (recursive)
        {
          if (remove_tree (argv[i]) != 0)
            rc = EXIT_FAILURE;
        }
      else if (remove (argv[i]) != 0)
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
  printf ("rm, remove a file\n");
  printf ("usage: rm FILE\n");
}
