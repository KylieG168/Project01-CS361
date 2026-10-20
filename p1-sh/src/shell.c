#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "cmd.h"
#include "effects.h"
#include "hash.h"
#include "model.h"
#include "process.h"

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

              if (next_token (&lookahead, next, sizeof (next))
                  && strcmp (next, ">") != 0 && strcmp (next, "|") != 0
                  && strcmp (next, "&") != 0)
                {
                  event = TOKEN;
                }

              if (fsm->nargs > 0)
                {
                  const char *previous = fsm->args[fsm->nargs - 1];
                  if (strcmp (previous, "-n") == 0
                      || strcmp (previous, "-f") == 0)
                    {
                      event = TOKEN;
                    }
                }
            }

          fsm->current_token = token;
          parsing = handle_event (fsm, event);
          if (fsm->parse_failed)
            parsing = false;
        }

      if (parsing)
        {
          fsm->current_token = "NL";
          handle_event (fsm, NEWLINE);
        }

      if (!fsm->debug && !fsm->parse_failed && fsm->commands != NULL
          && fsm->commands->next != NULL && fsm->commands->next->next == NULL)
        {
          int rc = run_pipe (fsm->commands->argv, fsm->commands->next->argv);
          char status_text[16];
          snprintf (status_text, sizeof (status_text), "%d", rc);
          hash_insert ("?", status_text);
        }

      if (!fsm->parse_failed && fsm->commands != NULL
          && fsm->commands->next == NULL)
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
              rc = run_args_redirect (fsm->result, fsm->commands->stdout_file);
            }
          char status_text[16];
          snprintf (status_text, sizeof (status_text), "%d", rc);
          hash_insert ("?", status_text);
        }
      if (fsm->result != NULL)
        {
          // for (size_t i = 0; fsm->result[i] != NULL; i++)
          //   free (fsm->result[i]);

          // free (fsm->result);
          parsed_command_t *node = fsm->commands;

          while (node != NULL)
            {
              parsed_command_t *next = node->next;

              for (size_t i = 0; node->argv[i] != NULL; i++)
                free (node->argv[i]);

              free (node->argv);
              free (node->stdout_file);
              free (node->stderr_file);
              free (node);

              node = next;
            }
        }

      free (fsm->stdout_file);
      free (fsm->stderr_file);
      if (fsm->args != NULL)
        {
          for (size_t i = 0; i < fsm->nargs; i++)
            free (fsm->args[i]);

          free (fsm->args);
        }
      free (fsm->command);
      free (fsm);
    }
  printf ("\n");
  hash_destroy ();
}
