#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "effects.h"
#include "model.h"

// TODO: Complete this table mapping state/events to the target state. Each
// row corresponds to the current state. Each column within that row indicates
// the next state for a given event. If there is no transition defined, that
// entry should be NST ("no state").
static state_t const _transitions[NUM_STATES][NUM_EVENTS] = {
  { Command, Term, NST },         // Init
  { Arguments, Make_Pipe, Term }, // Command
  { Arguments, Make_Pipe, Term }, // Arguments
  { Command, Term, Term },        // Make_Pip
  { NST, NST, NST }               // TERM
};

// TODO: Create a table mapping states/events to the effect functions. If
// there is no valid transition, the entry here would be NULL because actions
// are function pointers.
static action_t const _effects[NUM_STATES][NUM_EVENTS] = {
  { start_command, error_pipe, NULL },          // Init
  { append, link_commands, execute },           // Command
  { append, link_commands, execute },           // Arguments
  { start_command, error_pipe, error_newline }, // Make_Pip
  { NULL, NULL, NULL }                          // TERM
};

static state_t cmdline_transition (fsm_t *fsm, event_t event,
                                   action_t *effect);

/* Create an instance of an FSM and initialize its fields as appropriate.
  Some fields are common to most FSMs (such as an initial state or a
  pointer to a transition function). Other fields will be specific to
  this fsm_t declaration. Return NULL if any part of the initialization
  fails. */

fsm_t *
cmdline_init (void)
{
  fsm_t *fsm = calloc (1, sizeof (fsm_t));
  if (fsm == NULL)
    {
      return NULL;
    }
  fsm->state = Init;
  fsm->transition = cmdline_transition;
  fsm->command = NULL;
  fsm->nargs = 0;
  fsm->current_token = NULL;
  return fsm;
}

// TODO: Create a transition function that is specific to this type
// of FSM. This function needs to take an fsm_t* and an event, returning
// both the new state and the effect to perform (the latter is returned
// using a call-by-reference parameter. This function should NOT contain
// any "if" types of statements based on the state or event; it should
// simply lookup these values in the tables defined above.
state_t
cmdline_transition (fsm_t *fsm, event_t event, action_t *effect)
{
  assert (fsm->state <= NST);
  assert (event <= NIL);
  assert (event < NIL);

  state_t next = _transitions[fsm->state][event];
  *effect = (next != NST) ? _effects[fsm->state][event] : NULL;

  return next;
}

/* Helper function for providing a printable string name for an event */
const char *
event_name (event_t evt)
{
  assert (evt <= NIL);

  // Event names for printing out
  const char *names[] = { "TOKEN", "PIPE", "NEWLINE", "NIL" };
  return names[evt];
}

/* Helper function for providing a printable string name for an state */
const char *
state_name (state_t st)
{
  assert (st <= NST);

  // State names for printing out
  const char *names[]
      = { "Init", "Command", "Arguments", "Make_Pipe", "Term", "NST" };
  return names[st];
}

/* Generic front-end for handling events. Should do nothing more
  than calling the FSM's transition function, performing an effect
  (if appropriate) and updating the state. Return false if the new
   state is the terminal state. */
bool
handle_event (fsm_t *fsm, event_t event)
{
  // TODO: Look up the current state/event combination in the
  // transition table. Print the following line for debugging
  // purposes just for this lab. This should be printed even
  // if there is no transition.
  //   printf ("[%s.%s -> %s]\n", ...);
  action_t effect;
  state_t next = cmdline_transition (fsm, event, &effect);
  if (fsm->debug)
    {
      printf ("[%s.%s -> %s]\n", state_name (fsm->state), event_name (event),
              state_name (next));
    }
  // TODO: If the state/event combination is valid, execute
  // the transition and effect function (if there is one).
  // If the next state is Term (terminated), return false.
  // Otherwise return true.

  if (next != NST)
    {
      fsm->state = next;
      if (effect != NULL)
        {
          effect (fsm);
        }
    }
  return fsm->state != Term;
}

/* Given a string, return the event type. Do not modify this function. */
event_t
lookup (char *token)
{
  if (!strcmp (token, "|"))
    return PIPE;

  if (!strcmp (token, "NL"))
    return NEWLINE;

  return TOKEN;
}
