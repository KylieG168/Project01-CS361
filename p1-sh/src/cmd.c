// Implementation of a command-line parser as a finite state machine.
// The functions in this file are derived from the lab 2 parser and provide
// the generic interface for parsing the command line only. All execution
// of the lines should be done elsewhere and not part of the FSM.

#include "cmd.h"
#include "effects.h"
#include "model.h"
#include <stdio.h>
#include <string.h>

// You should provide some function like this to serve as the interface to
// parsing the command line. This function is just a placeholder and you
// should define your own.

// Returns the line without leading blanks or the trailing newline.
char *
parse_buffer (char *buffer)
{
  char *command = buffer;

  while (*command == ' ' || *command == '\t')
    command++;

  size_t len = strlen (command);
  while (len > 0 && (command[len - 1] == '\n' || command[len - 1] == '\r'))
    command[--len] = '\0';

  return command;
}

event_t
lookup (char *token)
{
  if (!strcmp (token, "|"))
    return PIPE;

  if (!strcmp (token, "NL"))
    return NEWLINE;

  if (!strcmp (token, "&"))
    return AMPERSAND;

  if (!strcmp (token, "1") || !strcmp (token, "2"))
    return FHANDLE;

  if (!strcmp (token, ">"))
    return REDIRECT;

  return TOKEN;
}

bool
next_token (const char **cursor, char *token, size_t capacity)
{
  const char *p = *cursor;
  while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
    p++;

  if (*p == '\0')
    {
      *cursor = p;
      return false;
    }

  size_t length = 0;

  if (*p == '|' || *p == '>' || *p == '&')
    {
      token[length++] = *p++;
    }
  else
    {
      while (*p != '\0' && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n'
             && *p != '|' && *p != '>' && *p != '&')
        {
          if (length + 1 >= capacity)
            return false;

          token[length++] = *p++;
        }
    }
  token[length] = '\0';
  *cursor = p;
  return true;
}
