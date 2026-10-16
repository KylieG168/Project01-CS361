#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "effects.h"
#include "model.h"
#include "effects.c"

static state_t const _transitions[NUM_STATES][NUM_EVENTS] = {
  { Command, Term, NST },         // Init
  { Arguments, Make_Pipe, Term }, // Command
  { Arguments, Make_Pipe, Term }, // Arguments
  { Command, Term, Term },        // Make_Pipe
  { NST, NST, NST }               // TERM
};

static action_t const _effects[NUM_STATES][NUM_EVENTS] = {
  { start_command, error_pipe, NULL },          // Init
  { append, link_commands, execute },           // Command
  { append, link_commands, execute },           // Arguments
  { start_command, error_pipe, error_newline }, // Make_Pipe
  { NULL, NULL, NULL }                          // TERM
};

const char *state_name(state_t state) {
    static const char *names[] = {
        "Init", "Command", "Arguments", "Make_Pipe", "Term", "NST"
    };
    if (state < 0 || state > NST) return "UNKNOWN";
    return names[state];
}

const char *event_name(event_t event) {
    static const char *names[] = {
        "TOKEN", "PIPE", "NEWLINE", "NIL"
    };
    if (event < 0 || event > NIL) return "UNKNOWN";
    return names[event];
}

state_t cmdline_transition(fsm_t *, event_t, action_t *);

fsm_t *cmdline_init(void) {
    fsm_t *fsm = calloc(1, sizeof(fsm_t));
    if (!fsm) return NULL;
    fsm->state = Init;
    fsm->transition = cmdline_transition;
    fsm->command = NULL;
    fsm->nargs = 0;
    fsm->current_token = NULL;
    return fsm;
}

state_t cmdline_transition(fsm_t *fsm, event_t event, action_t *effect) {
    assert(fsm->state < NST);
    assert(event < NIL);
    state_t next = _transitions[fsm->state][event];
    *effect = (next != NST) ? _effects[fsm->state][event] : NULL;
    return next;
}

bool handle_event(fsm_t *fsm, event_t event) {
    action_t effect;
    state_t next = cmdline_transition(fsm, event, &effect);
    if (next != NST) {
        fsm->state = next;
        if (effect) effect(fsm);
    }
    return fsm->state != Term;
}

