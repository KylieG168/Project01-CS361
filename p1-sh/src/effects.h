#ifndef __effects_h__
#define __effects_h__

#include "model.h"

void start_command (fsm_t *);
void append (fsm_t *);
void execute (fsm_t *);
void link_commands (fsm_t *);
void error_pipe (fsm_t *);
void error_newline (fsm_t *);

#endif
