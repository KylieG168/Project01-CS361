#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static void usage (void);

static int
touch_file (const char *path)
{
  FILE *fp = fopen (path, "r+");
  if (fp == NULL)
    {
      if (errno != ENOENT)
        return -1;
      fp = fopen (path, "w");
      if (fp == NULL)
        return -1;
      return fclose (fp);
    }

  int c = fgetc (fp);
  if (c == EOF)
    {
      if (ferror (fp))
        {
          fclose (fp);
          return -1;
        }
      fclose (fp);
      fp = fopen (path, "w");
      if (fp == NULL)
        return -1;
      return fclose (fp);
    }

  if (fseek (fp, 0, SEEK_SET) != 0 || fputc (c, fp) == EOF)
    {
      fclose (fp);
      return -1;
    }
  return fclose (fp);
}


int
main (int argc, char *argv[])
{
  if (argc < 2)
    {
      usage ();
      return EXIT_FAILURE;
    }

  int rc = EXIT_SUCCESS;
  for (int i = 1; i < argc; i++)
    {
      if (touch_file (argv[i]) != 0)
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
  printf ("touch, create an empty file or update its timestamp\n");
  printf ("usage: touch FILE...\n");
}
