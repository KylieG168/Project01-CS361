#include <stdio.h>
#include <string.h>

#include "builtins.h"
#include "cmd.h"
#include "hash.h"
#include "process.h"

// No command line can be more than 100 characters
#define MAXLENGTH 100

static bool
is_cmd (const char *line, const char *name)
{
  size_t n = strlen (name);
  return strncmp (line, name, n) == 0
         && (line[n] == ' ' || line[n] == '\n' || line[n] == '\0');
}

void
shell (FILE *input)
{
  hash_init (100);
  hash_insert ("?", "0");
  char buffer[MAXLENGTH];

  while (1)
    {
      // Print the cursor and get the next command entered
      printf ("$ ");
      memset (buffer, 0, sizeof (buffer));
      if (fgets (buffer, MAXLENGTH, input) == NULL)
        break;

      if (input != stdin)
        printf ("%s", buffer);

      // Keep this here to avoid weird line interleavings
      fflush (stdout);

      // Your code should begin here...
      if (buffer[0] == '\n' || buffer[0] == '\0')
        {
          continue;
        }
      else if (is_cmd (buffer, "quit"))
        break;
      else if (is_cmd (buffer, "echo"))
        echo (&buffer[5]);
      else
        {
          char *cmd = parse_buffer (buffer);
          int rc = run_process (cmd);
          char status_text[16];
          snprintf (status_text, sizeof (status_text), "%d", rc);
          hash_insert ("?", status_text);
        }
    }
  printf ("\n");
  hash_destroy ();
}
