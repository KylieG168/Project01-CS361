#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>

static void usage (void);

// WARNING WARNING WARNING:
// When using opendir and readdir to read directory listings, the files
// are returned in the order they were created. There is randomness to that
// creation that can cause tests to fail. You MUST sort the file names as
// expected in the test case files. To avoid making your task harder than
// it needs to be, we strongly suggest you use scandir instead.

static int
compare_names (const struct dirent **left, const struct dirent **right)
{
  const char *a = (*left)->d_name;
  const char *b = (*right)->d_name;

  while (*a == '.')
    a++;
  while (*b == '.')
    b++;

  int result = strcasecmp (a, b);
  if (result == 0)
    return strcmp ((*left)->d_name, (*right)->d_name);

  return result;
}

static void
print_permissions (mode_t mode)
{
  char permissions[11] = "----------";

  if (S_ISDIR (mode))
    permissions[0] = 'd';

  if (mode & S_IRUSR) permissions[1] = 'r';
  if (mode & S_IWUSR) permissions[2] = 'w';
  if (mode & S_IXUSR) permissions[3] = 'x';

  if (mode & S_IRGRP) permissions[4] = 'r';
  if (mode & S_IWGRP) permissions[5] = 'w';
  if (mode & S_IXGRP) permissions[6] = 'x';

  if (mode & S_IROTH) permissions[7] = 'r';
  if (mode & S_IWOTH) permissions[8] = 'w';
  if (mode & S_IXOTH) permissions[9] = 'x';

  printf ("%s ", permissions);
}

int
main (int argc, char *argv[])
{
  bool show_all = false;
  bool show_permissions = false;
  bool show_sizes = false;

  int option;
  while ((option = getopt (argc, argv, "aps")) != -1)
    {
      switch (option)
        {
          case 'a':
            show_all = true;
            break;

          case 'p':
            show_permissions = true;
            break;

          case 's':
            show_sizes = true;
            break;

          default:
            usage ();
            return EXIT_FAILURE;
        }
    }

  const char *directory = ".";
  if (optind < argc)
    directory = argv[optind];

  struct dirent **entries = NULL;
  int count = scandir (directory, &entries, NULL, compare_names);

  if (count == -1)
    {
      return EXIT_FAILURE;
    }

  int result = EXIT_SUCCESS;

  for (int i = 0; i < count; i++)
    {
      const char *name = entries[i]->d_name;
      bool is_dot_entry = strcmp (name, ".") == 0
                          || strcmp (name, "..") == 0;

      if (!is_dot_entry && (show_all || name[0] != '.'))
        {
          // printf ("%s\n", name);
          size_t path_length = strlen (directory) + strlen (name) + 2;
          char *path = malloc (path_length);

          if (path == NULL)
            {
              result = EXIT_FAILURE;
            }
          else
            {
              snprintf (path, path_length, "%s/%s", directory, name);

              struct stat info;
              if (stat (path, &info) == -1)
                {
                  result = EXIT_FAILURE;
                }
              else if (!(show_sizes && S_ISDIR (info.st_mode)))
                {
                  if (show_permissions)
                    print_permissions (info.st_mode);

                  if (show_sizes)
                    printf ("%lld ", (long long) info.st_size);

                  printf ("%s\n", name);
                }
              free (path);
            }
        }
      free (entries[i]);
    }
  free (entries);
  return result;
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
