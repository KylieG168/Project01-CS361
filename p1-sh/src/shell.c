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

static char *
expand_variables (const char *text)
{
  size_t capacity = strlen (text) + 1;
  size_t used = 0;
  char *output = malloc (capacity);

  if (output == NULL)
    return NULL;

  const char *p = text;

  while (*p != '\0')
    {
      if (p[0] == '$' && p[1] == '{')
        {
          const char *end = strchr (p + 2, '}');

          if (end != NULL)
            {
              size_t length = (size_t)(end - (p + 2));
              char *name = malloc (length + 1);
              if (name == NULL)
                {
                  free (output);
                  return NULL;
                }
              memcpy (name, p + 2, length);
              name[length] = '\0';

              const char *value = hash_find (name);
              free (name);

              if (value == NULL)
                value = "";

              size_t value_length = strlen (value);
              size_t needed = used + value_length + 1;

              if (needed > capacity)
                {
                  char *grown = realloc (output, needed);
                  if (grown == NULL)
                    {
                      free (output);
                      return NULL;
                    }

                  output = grown;
                  capacity = needed;
                }

              memcpy (output + used, value, value_length);
              used += value_length;
              p = end + 1;
              continue;
            }
        }
      if (used + 2 > capacity)
        {
          char *grown = realloc (output, used + 2);
          if (grown == NULL)
            {
              free (output);
              return NULL;
            }

          output = grown;
          capacity = used + 2;
        }

      output[used++] = *p++;
    }

  output[used] = '\0';
  return output;
}

typedef struct saved_variable
{
  char *name;
  char *old_value;
} saved_variable_t;

static void
restore_variables (saved_variable_t *saved, size_t count)
{
  while (count > 0)
    {
      count--;

      if (saved[count].old_value != NULL)
        hash_insert (saved[count].name, saved[count].old_value);
      else
        hash_remove (saved[count].name);

      free (saved[count].name);
      free (saved[count].old_value);
    }
}

void
shell (FILE *input)
{
  hash_init (100);
  hash_insert ("?", "0");
  const char *path = getenv ("PATH");
  if (path == NULL)
    path = "/usr/bin:/bin";

  hash_insert ("PATH", (char *)path);
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
          saved_variable_t saved[MAXLENGTH] = { 0 };
          size_t assignment_count = 0;
          bool assignments_ok = true;

          while (fsm->result[assignment_count] != NULL)
            {
              char *argument = fsm->result[assignment_count];
              char *equals = strchr (argument, '=');

              if (equals == NULL || equals == argument)
                break;

              size_t length = (size_t)(equals - argument);
              char *name = malloc (length + 1);

              if (name == NULL)
                {
                  assignments_ok = false;
                  break;
                }

              memcpy (name, argument, length);
              name[length] = '\0';

              const char *previous = hash_find (name);
              char *old_value = previous != NULL ? strdup (previous) : NULL;

              if (previous != NULL && old_value == NULL)
                {
                  free (name);
                  assignments_ok = false;
                  break;
                }

              saved[assignment_count].name = name;
              saved[assignment_count].old_value = old_value;
              assignment_count++;

              if (!hash_insert (name, equals + 1))
                {
                  assignments_ok = false;
                  break;
                }
            }

          char **command_argv = fsm->result + assignment_count;
          int rc = assignments_ok ? 0 : 1;

          if (assignments_ok && command_argv[0] != NULL)
            {
              for (size_t i = 1; command_argv[i] != NULL; i++)
                {
                  if (!strcmp (command_argv[i], "$?"))
                    {
                      const char *code = hash_find ("?");
                      char *replacement = strdup (code != NULL ? code : "");

                      if (replacement == NULL)
                        continue;

                      free (command_argv[i]);
                      command_argv[i] = replacement;
                    }
                }

              char *cmd = command_argv[0];

              if (!strcmp (cmd, "echo"))
                {
                  for (size_t i = 1; command_argv[i] != NULL; i++)
                    {
                      char *expanded = expand_variables (command_argv[i]);

                      if (expanded == NULL)
                        {
                          rc = 1;
                          break;
                        }
                      free (command_argv[i]);
                      command_argv[i] = expanded;
                    }
                  char line[MAXLENGTH] = "";
                  const char *delim = " ";
                  size_t first = 1;

                  if (command_argv[1] != NULL
                      && strncmp (command_argv[1], "-d", 2) == 0)
                    {
                      delim = command_argv[1] + 2;
                      first = 2;
                    }

                  for (size_t i = first; command_argv[i] != NULL; i++)
                    {
                      if (i > first)
                        strncat (line, delim, sizeof line - strlen (line) - 1);
                      strncat (line, command_argv[i],
                               sizeof line - strlen (line) - 1);
                    }
                  echo (line);
                  rc = 0;
                }
              else if (!strcmp (cmd, "cd"))
                {
                  rc = cd (command_argv[1] ? command_argv[1] : "");
                }
              else if (!strcmp (cmd, "pwd"))
                {
                  rc = pwd (NULL);
                }
              else if (!strcmp (cmd, "type"))
                {
                  for (size_t i = 1; command_argv[i] != NULL; i++)
                    {
                      if (type (command_argv[i]) != 0)
                        rc = 1;
                    }
                }
              else if (!strcmp (cmd, "export"))
                {
                  if (command_argv[1] == NULL)
                    rc = 1;

                  for (size_t i = 1; command_argv[i] != NULL; i++)
                    {
                      if (export_var (command_argv[i]) != 0)
                        rc = 1;
                    }
                }
              else if (!strcmp (cmd, "unset"))
                {
                  if (command_argv[1] == NULL)
                    rc = 1;

                  for (size_t i = 1; command_argv[i] != NULL; i++)
                    {
                      if (unset_var (command_argv[i]) != 0)
                        rc = 1;
                    }
                }
              else
                {
                  rc = run_args_redirect (command_argv,
                                          fsm->commands->stdout_file);
                }
            }

          restore_variables (saved, assignment_count);

          char status_text[16];
          snprintf (status_text, sizeof (status_text), "%d", rc);
          hash_insert ("?", status_text);
        }
      if (fsm->commands != NULL)
        {
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
