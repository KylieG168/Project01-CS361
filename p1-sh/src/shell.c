#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "builtins.h"
#include "cmd.h"
#include "hash.h"
#include "process.h"
#include "model.h"
#include "effects.h"

int mqopen (char **args);
int mqsend (char **args);
int mqrecv (char **args);
int mqclose (char **args);
void mq_close_all (void);
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

static char *
expand_vars (const char *word)
{
  if (strchr (word, '$') == NULL)
    return NULL;

  size_t cap = MAXLENGTH * 4;
  char *out = malloc (cap);
  if (out == NULL)
    return NULL;
  size_t n = 0;

  for (size_t i = 0; word[i] != '\0' && n + 1 < cap; i++)
    {
      const char *value = NULL;
      size_t skip = 0;

      if (word[i] == '$' && word[i + 1] == '?')
        {
          value = hash_find ("?");
          skip = 1;
        }
      else if (word[i] == '$' && word[i + 1] == '{')
        {
          const char *close = strchr (word + i + 2, '}');
          if (close != NULL)
            {
              char name[64];
              size_t len = (size_t) (close - (word + i + 2));
              if (len > 0 && len < sizeof name)
                {
                  memcpy (name, word + i + 2, len);
                  name[len] = '\0';
                  value = hash_find (name);
                  skip = len + 2;       /* "{" + name + "}" */
                  if (value == NULL)
                    value = "";
                }
            }
        }

      if (skip == 0)
        out[n++] = word[i];
      else
        {
          if (value == NULL)
            value = "";
          size_t vlen = strlen (value);
          if (n + vlen + 1 >= cap)
            break;
          memcpy (out + n, value, vlen);
          n += vlen;
          i += skip;
        }
    }
  out[n] = '\0';
  return out;
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
      char *save = NULL;

      for (char *tok = strtok_r (buffer, " \t\r\n", &save);
           tok != NULL;
           tok = strtok_r (NULL, " \t\r\n", &save))
        {
          fsm->current_token = tok;
          handle_event (fsm, lookup (tok));
        }

      fsm->current_token = "NL";
      handle_event (fsm, NEWLINE);

      if (fsm->result)
        {
          for (size_t i = 1; fsm->result[i] != NULL; i++)
            {
              char *expanded = expand_vars (fsm->result[i]);
              if (expanded != NULL)
                {
                  free (fsm->result[i]);
                  fsm->result[i] = expanded;
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
          else if (!strcmp (cmd, "mqopen"))
            {
              rc = mqopen (fsm->result);
            }
          else if (!strcmp (cmd, "mqsend"))
            {
              rc = mqsend (fsm->result);
            }
          else if (!strcmp (cmd, "mqrecv"))
            {
              rc = mqrecv (fsm->result);
            }
          else if (!strcmp (cmd, "mqclose"))
            {
              rc = mqclose (fsm->result);
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
  mq_close_all ();
  printf ("\n");
  hash_destroy ();
}
