// Implementation of a command-line parser as a finite state machine.
// The functions in this file are derived from the lab 2 parser and provide
// the generic interface for parsing the command line only. All execution
// of the lines should be done elsewhere and not part of the FSM.

#include <stdio.h>
#include <string.h>

#include "cmd.h"

// You should provide some function like this to serve as the interface to
// parsing the command line. This function is just a placeholder and you
// should define your own.
char *
parse_buffer (char *buffer)
{
    char *command = strtok (buffer, " ");
    return command;
}
