// Implementation of a command-line parser as a finite state machine.
// The functions in this file are derived from the lab 2 parser and provide
// the generic interface for parsing the command line only. All execution
// of the lines should be done elsewhere and not part of the FSM.

#include <stdio.h>
#include <string.h>

#include "cmd.h"
#include "effects.h"
#include "model.h"

// You should provide some function like this to serve as the interface to
// parsing the command line. This function is just a placeholder and you
// should define your own.

// Returns the line without leading blanks or the trailing newline.
char *
parse_buffer (char *buffer)
{
    char *command = buffer;
    while (*command == ' ' || *command == '\t')
        {
            command++;
        }

    size_t len = strlen (command);
    while (len > 0 && (command[len - 1] == '\n' || command[len - 1] == '\r'))
        {
            command[--len] = '\0';
        }
    return command;
}

