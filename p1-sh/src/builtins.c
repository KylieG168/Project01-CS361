#include "builtins.h"
#include "hash.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE 100

#define MQ_MAX_QUEUES 10
#define MQ_MAX_MSGS 10
#define MQ_DEFAULT_PRIO 10

typedef struct
{
  char *text;
  long prio;
  unsigned long seq;
} msg_t;

typedef struct
{
  char *name;
  bool open;
  msg_t msgs[MQ_MAX_MSGS];
  size_t count;
} msgq_t;

int
echo (char *message)
{
  size_t len = strnlen (message, MAX_LINE);
  if (len > 0 && message[len - 1] == '\n')
    message[len - 1] = '\0';

  for (int i = 0; message[i] != '\0'; i++)
    {
      if (message[i] == '\\' && message[i + 1] == 'n')
        {
          printf ("\n");
          i++;
        }
      else
        {
          printf ("%c", message[i]);
        }
    }

  printf ("\n");
  return 0;
}

int
cd (char *message)
{
  while (*message == ' ' || *message == '\t')
    {
      message++;
    }

  size_t len = strnlen (message, MAX_LINE);
  if (len > 0 && message[len - 1] == '\n')
    message[len - 1] = '\0';

  if (message[0] == '\0')
    {
      return 0;
    }
  if (chdir (message) != 0)
    {
      perror (message);
      return -1;
    }
  return 0;
}

int
pwd (char *message)
{
  (void)message;
  char cwd[4096];

  if (getcwd (cwd, sizeof cwd) == NULL)
    {
      perror ("pwd");
      return 1;
    }

  printf ("%s\n", cwd);
  return 0;
}

static msgq_t queues[MQ_MAX_QUEUES];
static size_t queue_count = 0;
static unsigned long send_seq = 0;

static bool
parse_int (const char *s, long *out)
{
  char *end = NULL;
  *out = strtol (s, &end, 10);
  return end != s && *end == '\0';
}

static msgq_t *
find_queue (const char *idstr)
{
  long id;
  if (idstr == NULL || !parse_int (idstr, &id))
    return NULL;
  if (id < 0 || (size_t)id >= queue_count)
    return NULL;
  return queues[id].open ? &queues[id] : NULL;
}

static char *
join_words (char **words)
{
  size_t len = 1;
  for (size_t i = 0; words[i] != NULL; i++)
    len += strlen (words[i]) + 1;

  char *out = malloc (len);
  if (out == NULL)
    return NULL;
  out[0] = '\0';
  for (size_t i = 0; words[i] != NULL; i++)
    {
      if (i > 0)
        strncat (out, " ", len - strlen (out) - 1);
      strncat (out, words[i], len - strlen (out) - 1);
    }
  return out;
}

static char *
take_best (msgq_t *q)
{
  size_t best = 0;

  for (size_t i = 1; i < q->count; i++)
    {
      if (q->msgs[i].prio < q->msgs[best].prio
          || (q->msgs[i].prio == q->msgs[best].prio
              && q->msgs[i].seq < q->msgs[best].seq))
        {
          best = i;
        }
    }

  char *text = q->msgs[best].text;
  for (size_t i = best; i + 1 < q->count; i++)
    {
      q->msgs[i] = q->msgs[i + 1];
    }
  q->count--;
  return text;
}

static void
close_queue (size_t id)
{
  msgq_t *q = &queues[id];

  if (q->count > 0)
    {
      printf ("[%zu] mq.%s not empty:\n", id, q->name);
      while (q->count > 0)
        {
          char *text = take_best (q);
          printf ("    '%s'\n", text);
          free (text);
        }
    }
  printf ("[%zu] mq.%s closed\n", id, q->name);

  free (q->name);
  q->name = NULL;
  q->open = false;
}

int
mqopen (char **args)
{
  if (args[1] == NULL)
    {
      fprintf (stderr, "usage: mqopen NAME\n");
      return 1;
    }
  if (queue_count >= MQ_MAX_QUEUES)
    {
      fprintf (stderr, "mqopen: too many queues\n");
      return 1;
    }

  msgq_t *q = &queues[queue_count];
  q->name = strdup (args[1]);
  if (q->name == NULL)
    return 1;
  q->open = true;
  q->count = 0;
  queue_count++;
  printf ("[%zu] mq.%s\n", queue_count - 1, q->name);
  return 0;
}

int
mqsend (char **args)
{
  size_t i = 1;
  long prio = MQ_DEFAULT_PRIO;

  if (args[i] != NULL && strcmp (args[i], "-p") == 0)
    {
      if (args[i + 1] == NULL || !parse_int (args[i + 1], &prio))
        {
          fprintf (stderr, "usage: mqsend [-p PRIORITY] ID MESSAGE\n");
          return 1;
        }
      i += 2;
    }

  msgq_t *q = find_queue (args[i]);
  if (q == NULL)
    {
      fprintf (stderr, "mqsend: no such queue\n");
      return 1;
    }
  if (q->count >= MQ_MAX_MSGS)
    {
      fprintf (stderr, "mqsend: queue is full\n");
      return 1;
    }

  char *text = join_words (&args[i + 1]);
  if (text == NULL)
    return 1;

  q->msgs[q->count].text = text;
  q->msgs[q->count].prio = prio;
  q->msgs[q->count].seq = send_seq++;
  q->count++;
  return 0;
}

int
mqrecv (char **args)
{
  if (args[1] == NULL || args[2] == NULL)
    {
      fprintf (stderr, "usage: mqrecv ID VARIABLE\n");
      return -1;
    }

  msgq_t *q = find_queue (args[1]);
  if (q == NULL)
    {
      fprintf (stderr, "mqrecv: no such queue\n");
      return -1;
    }
  if (q->count == 0)
    {
      hash_insert (args[2], "");
      return -1;
    }

  char *text = take_best (q);
  hash_insert (args[2], text);
  free (text);
  return 0;
}

int
mqclose (char **args)
{
  msgq_t *q = find_queue (args[1]);
  if (q == NULL)
    {
      fprintf (stderr, "mqclose: no such queue\n");
      return 1;
    }
  close_queue ((size_t)(q - queues));
  return 0;
}

void
mq_close_all (void)
{
  for (size_t i = 0; i < queue_count; i++)
    {
      if (queues[i].open)
        close_queue (i);
    }
}

int
export_var (char *assignment)
{
  char *equals = strchr (assignment, '=');

  if (equals == NULL || equals == assignment)
    return 1;

  *equals = '\0';
  bool inserted = hash_insert (assignment, equals + 1);
  *equals = '=';

  return inserted ? 0 : 1;
}

int
unset_var (char *name)
{
  if (name == NULL || name[0] == '\0')
    return 1;

  return hash_remove (name) ? 0 : 1;
}

int
type (char *message)
{
  size_t len = strnlen (message, MAX_LINE);
  if (len > 0 && message[len - 1] == '\n')
    {
      message[len - 1] = '\0';
    }
  if (message[0] == '\0')
    {
      return 0;
    }

  const char *names[] = { "cd",   "echo",   "pwd",  "quit",
                          "type", "export", "unset" }; // all the types
  for (size_t i = 0; i < sizeof names / sizeof names[0]; i++)
    {
      if (strcmp (message, names[i]) == 0) // if it is a builtin
        {
          printf ("%s is a shell builtin\n", message);
          return 0;
        }
    }

  const char *path = getenv ("PATH"); // if it is not a builtin
  if (path != NULL)
    {
      char *copy = strdup (path);
      if (copy == NULL)
        {
          perror ("strdup");
          return 1;
        }

      char *save = NULL;
      for (char *dir = strtok_r (copy, ":", &save); dir != NULL; // split at :
           dir = strtok_r (NULL, ":", &save))
        {
          char full[4096];
          snprintf (full, sizeof full, "%s/%s", dir, message);
          if (access (full, X_OK) == 0) // checks if it's possible to execute
            {
              printf ("%s is %s\n", message,
                      full); // prints if it is executable
              free (copy);
              return 0;
            }
        }
      free (copy); // frees the copy if it is not executable
    }
  printf ("%s not found\n", message);
  return 1;
}
