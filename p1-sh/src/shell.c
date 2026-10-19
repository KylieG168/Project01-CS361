#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "builtins.h"
#include "cmd.h"
#include "hash.h"
#include "process.h"
#include "model.h"
#include "effects.h"

extern bool fsm_debug;

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
      printf ("$ ");
      memset (buffer, 0, sizeof (buffer));
      if (fgets (buffer, MAXLENGTH, input) == NULL)
        break;

      if (input != stdin)
        printf ("%s", buffer);

      fflush (stdout);

      if (buffer[0] == '\n' || buffer[0] == '\0')
        {
          continue;
        }
      if (is_cmd (buffer, "quit"))
        {
          break;
        }

      fsm_t *fsm = cmdline_init ();
      if (fsm == NULL)
        break;
      fsm->debug = fsm_debug;

      const char *cursor = buffer;
      char token[MAXLENGTH];
      bool parsing = true;

      while (parsing && next_token (&cursor, token, sizeof (token)))
        {
          event_t event = lookup (token);
          if (event == FHANDLE)
            {
              const char *lookahead = cursor;
              char next[MAXLENGTH];

              if (next_token (&lookahead, next, sizeof (next)) && strcmp (next, ">") != 0 && strcmp (next, "|") != 0 && strcmp (next, "&") != 0)
                {
                  event = TOKEN;
                }
            }

          fsm->current_token = token;
          parsing = handle_event (fsm, event);
        }

      if (parsing)
        {
          fsm->current_token = "NL";
          handle_event (fsm, NEWLINE);
        }

      if (fsm->result)
        {
          for (size_t i = 1; fsm->result[i] != NULL; i++)
            {
              if (!strcmp (fsm->result[i], "$?"))
                {
                  char *replacement = strdup (hash_find ("?"));
                  if (replacement == NULL)
                    continue;
                  free (fsm->result[i]);
                  fsm->result[i] = replacement;
                }
            }

          char *cmd = fsm->result[0];
          int rc = 0;

          if (!strcmp (cmd, "echo"))
            {
              char line[MAXLENGTH] = "";
              const char *delim = " ";
              size_t first = 1;

              if (fsm->result[1] != NULL
                  && strncmp (fsm->result[1], "-d", 2) == 0)
                {
                  delim = fsm->result[1] + 2;
                  first = 2;
                }

              for (size_t i = first; fsm->result[i] != NULL; i++)
                {
                  if (i > first)
                    strncat (line, delim, sizeof line - strlen (line) - 1);
                  strncat (line, fsm->result[i],
                           sizeof line - strlen (line) - 1);
                }
              echo (line);
              rc = 0;
            }
          else if (!strcmp (cmd, "cd"))
            {
              rc = cd (fsm->result[1] ? fsm->result[1] : "");
            }
          else if (!strcmp (cmd, "pwd"))
            {
              rc = pwd (NULL);
            }
          else if (!strcmp (cmd, "type"))
            {
              for (size_t i = 1; fsm->result[i] != NULL; i++)
                {
                  if (type (fsm->result[i]) != 0)
                    rc = 1;
                }
            }
          else
            {
              rc = run_args (fsm->result);
            }
          char status_text[16];
          snprintf (status_text, sizeof (status_text), "%d", rc);
          hash_insert ("?", status_text);
        }
      if (fsm->result != NULL)
        {
          for (size_t i = 0; fsm->result[i] != NULL; i++)
            free (fsm->result[i]);

          free (fsm->result);
        }
      free (fsm);
    }
  printf ("\n");
  hash_destroy ();
}
