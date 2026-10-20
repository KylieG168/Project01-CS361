#ifndef __model_h__
#define __model_h__

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

// Generic definitions for any type of statemodel

// States and events should just be integers
typedef int state_t;
typedef int event_t;

// Needed for circular typedef. This lets action_t use fsm_t in its parameter
// list, while the struct fsm can use action_t as a field.
typedef struct fsm fsm_t;

// All entry, exit, and effect instances use the action type
typedef void (*action_t) (fsm_t *);

typedef struct parsed_command
{
  char **argv;
  char *stdout_file;
  char *stderr_file;
  bool merge_stderr;
  bool background;

  struct parsed_command *next;
} parsed_command_t;

// Each FSM instance contains a current state
struct fsm
{
  state_t state; // current state

  // pointer to the FSM's transition function
  state_t (*transition) (struct fsm *, event_t, action_t *);

  // Additional data fields specific to this FSM
  char *command;       // the name of the command to run
  size_t nargs;        // the number of command-line arguments
  char **args;         // the command-line arguments
  char *current_token; // current token being processed
  bool debug;
  char **result;
  bool background;
  int pending_number;
  int redirect_fd;
  char *stdout_file;
  char *stderr_file;
  bool merge_stderr;
  parsed_command_t *commands;
  parsed_command_t *last_command;
  bool parse_failed;
};

// Generic entry point for handling events
bool handle_event (fsm_t *, event_t);

// Additional definitions specific to an FSM for command line processing

#define MAX_ARGUMENTS 100

// Events
typedef enum
{
  TOKEN,   // normal command-line token
  PIPE,    // vertical bar character
  NEWLINE, // newline at the end of the command
  AMPERSAND,
  FHANDLE,
  REDIRECT,
  NIL // invalid non-event
} cmdevt_t;
#define NUM_EVENTS NIL

// States
typedef enum
{
  Init,      // initial state
  Command,   // establishing the command name
  Arguments, // building the argument list
  Make_Pipe, // linking the commands together for a pipe
  Background,
  Number,
  Redirecting,
  Redirection,
  Merging,
  Term, // terminal state (execute program or error)
  NST   // invalid non-state
} cmdst_t;
#define NUM_STATES NST

// Helper functions
fsm_t *cmdline_init (void); // initialize the FSM
event_t lookup (char *);    // convert an event string to its numeric value

// Translate event/state numbers to their string equivalent
const char *event_name (event_t);
const char *state_name (state_t);

#endif
