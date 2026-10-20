#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage (void);

static void usage (void);

static bool
is_elf (const unsigned char *h)
{
  return h[0] == 0x7f && h[1] == 'E' && h[2] == 'L' && h[3] == 'F';
}

static bool
is_big (const unsigned char *h)
{
  static const unsigned char magics[][4] = {
    { 0xfe, 0xed, 0xfa, 0xce }, { 0xfe, 0xed, 0xfa, 0xcf },
    { 0xce, 0xfa, 0xed, 0xfe }, { 0xcf, 0xfa, 0xed, 0xfe },
    { 0xca, 0xfe, 0xba, 0xbe }
  };
  for (size_t i = 0; i < sizeof magics / sizeof magics[0]; i++)
    {
      if (memcmp (h, magics[i], 4) == 0)
        return true;
    }
  return false;
}

static int
describe (const char *path)
{
  FILE *fp = fopen (path, "rb");
  if (fp == NULL)
    {
      perror (path);
      return 1;
    }

  unsigned char buf[4096];
  unsigned char head[8] = { 0 };
  size_t total = 0, n;
  bool ascii = true;

  while ((n = fread (buf, 1, sizeof buf, fp)) > 0)
    {
      for (size_t i = 0; i < n; i++)
        {
          if (total < sizeof head)
            head[total] = buf[i];
          total++;
          if (buf[i] > 127)
            ascii = false;
        }
    }
  if (ferror (fp))
    {
      perror (path);
      fclose (fp);
      return 1;
    }

  if (total == 0)
    printf ("%s: empty\n", path);
  else if (total >= 4 && is_elf (head))
    printf ("%s: ELF executable\n", path);
  else if (total >= 4 && is_big (head))
    printf ("%s: Mach-O executable\n", path);
  else if (total >= 2 && head[0] == '#' && head[1] == '!')
    {
      char line[256];
      rewind (fp);
      if (fgets (line, sizeof line, fp) != NULL)
        {
          line[strcspn (line, "\r\n")] = '\0';
          printf ("%s: script, interpreter %s\n", path, line + 2);
        }
    }
  else if (ascii)
    printf ("%s: ASCII text\n", path);
  else
    printf ("%s: binary data\n", path);

  fclose (fp);
  return 0;
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
      if (describe (argv[i]) != 0)
        rc = EXIT_FAILURE;
    }
  return rc;
}

static void __attribute__ ((unused))
usage (void)
{
  printf ("file, get a summary of information about a file\n");
  printf ("usage: file [-t] FILE...\n");
  printf ("  -t       include language type for source code files\n");
}
