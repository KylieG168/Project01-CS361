#include <stdio.h>
#include <string.h>

#include "builtins.h"
#include "cmd.h"
#include "hash.h"
#include "process.h"

// No command line can be more than 100 characters
#define MAXLENGTH 100

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
      char *cmd = parse_buffer (buffer);
      int rc __attribute__ ((unused)) = run_process (cmd);
    }
  printf ("\n");
  hash_destroy ();
}
