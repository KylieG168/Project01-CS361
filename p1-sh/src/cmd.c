// Implementation of a command-line parser as a finite state machine.
// The functions in this file are derived from the lab 2 parser and provide
// the generic interface for parsing the command line only. All execution
// of the lines should be done elsewhere and not part of the FSM.

#include <stdio.h>
#include <string.h>

#include "cmd.h"
#include "effects.h"
#include "model.c"

static bool debug_mode = false;
// You should provide some function like this to serve as the interface to
// parsing the command line. This function is just a placeholder and you
// should define your own.
void
cmd_set_debug (bool on)
{
    debug_mode = on;
}

bool
cmd_debug (void)
{
    return debug_mode;
}

void
free_args (char **args)
{
    if (args == NULL)
    {
        return;
    }
    for (size_t i = 0; args[i] != NULL; i++)
    {
        free (args[i]);
    }
    free (args);
}

char *
parse_buffer (char *buffer)
{
    fsm_t *fsm = cmdline_init ();
    if (fsm == NULL)
    {
        return NULL;
    }
    fsm->debug = debug_mode;
    char *save = NULL;
    bool more = true;
    char *tok = NULL;

    for (tok = strtok_r(buffer, " \t\r\n", &save);
        tok != NULL && more;
        tok = strtok_r(NULL, " \t\r\n", &save))
        {
            fsm->current_token = tok;
            more = handle_event(fsm, lookup(tok));
        }


    if (more)
    {
        fsm->current_token = "NL";
        handle_event (fsm, NEWLINE);
    }

    char *result = fsm->*result;
    if (fsm->args != NULL)
    {
            for (size_t i = 0; i < fsm->nargs; i++)
            {
                free (fsm->args[i]);
            }
            free (fsm->args);
    }
    free (fsm->command);
    free (fsm);
    return result;
}

