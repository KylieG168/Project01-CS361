#define _POSIX_C_SOURCE 200809L // needed for strdup extension

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "effects.h"
#include "model.h"

/* Executed when starting to process a new command line. The fsm_t
  should have been updated to include a pointer to the current token.
  For instance, if the command line was "ls -l data NL", the fsm_t
  has a field that points to "ls". */
void
start_command (fsm_t *cmdmodel)
{
  // TODO: Copy the current token to store it in the FSM's command
  // field. Next, create the FSM's args array (length MAX_ARGUMENTS)
  // set the current token as args[0], and initialize nargs to be
  // the number of arguments (1 at this point).
  cmdmodel->command
      = cmdmodel->current_token ? strdup (cmdmodel->current_token) : NULL;
  cmdmodel->args = calloc (MAX_ARGUMENTS + 1, sizeof (char *));
  cmdmodel->args[0]
      = cmdmodel->current_token ? strdup (cmdmodel->current_token) : NULL;
  cmdmodel->nargs = 1;
  cmdmodel->background = false;
}

/* Executed when processing a token after the command name. For instance,
  if the command line was "ls -l data NL", this function will be called
  when the current token is "-l" and again when it is "data". */
void
append (fsm_t *cmdmodel)
{
  if (cmdmodel->nargs >= MAX_ARGUMENTS)
    return;

  assert (cmdmodel->args != NULL);

  // TODO: Store the current token into the args array and increment nargs
  cmdmodel->args[cmdmodel->nargs]
      = cmdmodel->current_token ? strdup (cmdmodel->current_token) : NULL;
  cmdmodel->nargs++;
}

static void
flush_number (fsm_t *cmdmodel)
{
  if (cmdmodel->pending_number == 0)
    return;

  char number[2] = { (char)('0' + cmdmodel->pending_number), '\0' };

  char *saved_token = cmdmodel->current_token;
  cmdmodel->current_token = number;
  append (cmdmodel);
  cmdmodel->current_token = saved_token;

  cmdmodel->pending_number = 0;
}

void
remember_number (fsm_t *cmdmodel)
{
  flush_number (cmdmodel);
  cmdmodel->pending_number = cmdmodel->current_token[0] - '0';
}

void
append_after_number (fsm_t *cmdmodel)
{
  flush_number (cmdmodel);
  append (cmdmodel);
}

/* Executed when either a NL or | (pipe) is encountered. For instance, if
  the command line is "ls -l data NL", the current token will be "NL"; also,
  the FSM's args array should be complete, containing "ls", "-l", and "data",
  followed by several NULL pointers. */
void
execute (fsm_t *cmdmodel)
{
  flush_number (cmdmodel);
  assert (cmdmodel->args != NULL);

  // TODO: Print out the argument list similar to the format shown and free
  // the args array.
  if (cmdmodel->debug)
    {
      printf ("  Execute %s%s with arguments {", cmdmodel->command,
              cmdmodel->background ? " in background" : "");
      for (size_t i = 0; i < cmdmodel->nargs; i++)
        printf (" %s,", cmdmodel->args[i]);
      printf (" (null) }\n");

      for (size_t i = 0; i < cmdmodel->nargs; i++)
        free (cmdmodel->args[i]);

      free (cmdmodel->args);
    }
  else
    {
      cmdmodel->result = cmdmodel->args;
    }

  free (cmdmodel->command);
  cmdmodel->command = NULL;
  cmdmodel->args = NULL;
  cmdmodel->nargs = 0;
}

// No changes are needed to the effects below

void
link_commands (fsm_t *cmdmodel)
{
  if (cmdmodel->debug)
    printf ("  Create pipe for output of %s\n", cmdmodel->command);
  execute (cmdmodel);
}

void
error_pipe (fsm_t *cmdmodel)
{
  printf ("ERROR: Received token %s while in state %s\n",
          cmdmodel->current_token, state_name (cmdmodel->state));
}

void
error_newline (fsm_t *cmdmodel)
{
  printf ("ERROR: Received token %s while in state %s\n",
          cmdmodel->current_token, state_name (cmdmodel->state));
}

void
mark_background (fsm_t *cmdmodel)
{
  flush_number (cmdmodel);
  cmdmodel->background = true;
}
