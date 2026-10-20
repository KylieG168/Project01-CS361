#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define HEAD_SIZE 8

typedef struct
{
  unsigned char head[HEAD_SIZE];
  size_t total;
  bool ascii;
} scan_t;

static void usage (void);

static bool
scan_file (FILE *fp, scan_t *s)
{
  unsigned char buf[4096];
  size_t n;

  memset (s, 0, sizeof *s);
  s->ascii = true;
  while ((n = fread (buf, 1, sizeof buf, fp)) > 0)
    {
      for (size_t i = 0; i < n; i++)
        {
          if (s->total < HEAD_SIZE)
            s->head[s->total] = buf[i];
          s->total++;
          if (buf[i] > 127)
            s->ascii = false;
        }
    }
  return !ferror (fp);
}

static bool
is_elf (const unsigned char *h)
{
  return h[0] == 0x7f && h[1] == 'E' && h[2] == 'L' && h[3] == 'F';
}

static bool
macho_kind (const unsigned char *h, bool *is64, bool *little)
{
  static const unsigned char le32[4] = { 0xce, 0xfa, 0xed, 0xfe };
  static const unsigned char le64[4] = { 0xcf, 0xfa, 0xed, 0xfe };
  static const unsigned char be32[4] = { 0xfe, 0xed, 0xfa, 0xce };
  static const unsigned char be64[4] = { 0xfe, 0xed, 0xfa, 0xcf };

  *little = true;
  *is64 = false;
  if (memcmp (h, le32, 4) == 0)
    return true;
  if (memcmp (h, le64, 4) == 0)
    {
      *is64 = true;
      return true;
    }
  *little = false;
  if (memcmp (h, be32, 4) == 0)
    return true;
  if (memcmp (h, be64, 4) == 0)
    {
      *is64 = true;
      return true;
    }
  return false;
}

static unsigned int
read_u32 (const unsigned char *p, bool little)
{
  unsigned int a = p[0], b = p[1], c = p[2], d = p[3];
  if (little)
    return a | (b << 8) | (c << 16) | (d << 24);
  return (a << 24) | (b << 16) | (c << 8) | d;
}

static const char *
cpu_name (unsigned int cpu, bool is64)
{
  switch (cpu & 0xff)
    {
    case 12:
      return (cpu >> 24) != 0 ? "arm64" : "arm";
    case 7:
      return is64 ? "x86_64" : "i386";
    default:
      return "unknown";
    }
}

static void
shebang_interp (FILE *fp, char *out, size_t size)
{
  char line[256];
  size_t i = 2;
  size_t n = 0;

  out[0] = '\0';
  rewind (fp);
  if (fgets (line, sizeof line, fp) == NULL)
    return;
  while (line[i] == ' ' || line[i] == '\t')
    i++;
  while (line[i] != '\0' && line[i] != ' ' && line[i] != '\t'
         && line[i] != '\r' && line[i] != '\n' && n + 1 < size)
    out[n++] = line[i++];
  out[n] = '\0';
}

static bool
is_makefile (const char *path)
{
  const char *base = strrchr (path, '/');
  base = (base != NULL) ? base + 1 : path;
  return strcmp (base, "Makefile") == 0 || strcmp (base, "makefile") == 0;
}


static void
classify (const char *path, FILE *fp, const scan_t *s, char *out, size_t size)
{
  const unsigned char *h = s->head;
  bool is64, little;

  if (s->total >= 5 && is_elf (h))
    {
      if (h[4] == 2)
        snprintf (out, size, "%s", "ELF 64-bit");
      else if (h[4] == 1)
        snprintf (out, size, "%s", "ELF 32-bit");
      else
        snprintf (out, size, "%s", "ELF");
    }
  else if (s->total >= HEAD_SIZE && macho_kind (h, &is64, &little))
    {
      const char *cpu = cpu_name (read_u32 (h + 4, little), is64);
      if (is64)
        snprintf (out, size, "Mach-O 64-bit executable %s", cpu);
      else
        snprintf (out, size, "Mach-O executable %s", cpu);
    }
  else if (s->total >= 2 && h[0] == '#' && h[1] == '!')
    {
      char interp[128];
      shebang_interp (fp, interp, sizeof interp);
      snprintf (out, size, "%s text executable", interp);
    }
  else if (is_makefile (path))
    snprintf (out, size, "%s", "makefile script text");
  else
    snprintf (out, size, "%s", "unknown type");
}

static int
describe (const char *path, bool types)
{
  FILE *fp = fopen (path, "rb");
  if (fp == NULL)
    {
      perror (path);
      return 1;
    }

  scan_t s;
  if (!scan_file (fp, &s))
    {
      perror (path);
      fclose (fp);
      return 1;
    }

  const char *base = s.ascii ? "ASCII text" : "binary";
  if (s.total == 0)
    printf ("%s: empty\n", path);
  else if (!types)
    printf ("%s: %s\n", path, base);
  else
    {
      char detail[256];
      classify (path, fp, &s, detail, sizeof detail);
      printf ("%s: %s, %s\n", path, detail, base);
    }

  fclose (fp);
  return 0;
}

int
main (int argc, char *argv[])
{
  bool types = false;
  int ch;

  while ((ch = getopt (argc, argv, "t")) != -1)
    {
      if (ch == 't')
        types = true;
      else
        {
          usage ();
          return EXIT_FAILURE;
        }
    }
  if (optind >= argc)
    {
      usage ();
      return EXIT_FAILURE;
    }

  int rc = EXIT_SUCCESS;
  for (int i = optind; i < argc; i++)
    {
      if (describe (argv[i], types) != 0)
        rc = EXIT_FAILURE;
    }
  return rc;
}

static void
usage (void)
{
  printf ("file, get a summary of information about a file\n");
  printf ("usage: file [-t] FILE...\n");
  printf ("  -t       include language type for source code files\n");
}
