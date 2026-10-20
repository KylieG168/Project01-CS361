#ifndef __cs361_process__
#define __cs361_process__

int run_process (char *cmd);
int run_args (char **argv);
int run_args_redirect (char **argv, const char *stdout_file);
int run_pipe (char **left, char **right);

#endif
