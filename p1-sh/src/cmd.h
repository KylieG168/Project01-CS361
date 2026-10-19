#ifndef __cs361_cmd_h__
#define __cs361_cmd_h__
#include <stdbool.h>
#include <stddef.h>

char *parse_buffer (char *);
bool next_token (const char **cursor, char *token, size_t capacity);

#endif
