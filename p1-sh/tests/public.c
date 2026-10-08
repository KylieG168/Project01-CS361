#include <check.h>

#include "../src/hash.h"

START_TEST (C_hash_insert)
{
  char *strings[] = { "foo", "bar", "zoo", "yadda", "help", "goo", "hi", "boo",
    "nine", "ten" };

  hash_init (100);

  for (int i = 0; i < 10; i++)
    ck_assert (hash_insert (strings[i], strings[(i + 1) % 10]));

  for (int i = 0; i < 10; i++)
    {
      char *value = hash_find (strings[i]);
      ck_assert_str_eq (value, strings[(i + 1) % 10]);
    }
}
END_TEST

START_TEST (C_hash_find_missing)
{
  char *strings[] = { "foo", "bar", "zoo", "yadda", "help", "goo", "hi", "boo",
    "nine", "ten" };

  hash_init (100);

  for (int i = 0; i < 10; i++)
    ck_assert (hash_insert (strings[i], strings[(i + 1) % 10]));

  ck_assert (hash_find ("gobble") == NULL);
}
END_TEST

START_TEST (C_hash_remove)
{
  char *strings[] = { "foo", "bar", "zoo", "yadda", "help", "goo", "hi", "boo",
    "nine", "ten" };

  hash_init (100);

  for (int i = 0; i < 10; i++)
    ck_assert (hash_insert (strings[i], strings[(i + 1) % 10]));

  for (int i = 0; i < 5; i++)
    ck_assert (hash_remove (strings[i]));

  ck_assert_str_eq (hash_find ("goo"), "hi");

  for (int i = 5; i < 10; i++)
    ck_assert (hash_remove (strings[i]));

  ck_assert (hash_find ("goo") == NULL);
}
END_TEST

START_TEST (C_hash_replace)
{
  char *strings[] = { "foo", "bar", "zoo", "yadda", "help", "goo", "hi", "boo",
    "nine", "ten" };

  hash_init (100);

  for (int i = 0; i < 10; i++)
    ck_assert (hash_insert (strings[i], strings[(i + 1) % 10]));

  ck_assert (hash_insert ("yadda", "again?!"));
  ck_assert_str_eq (hash_find ("yadda"), "again?!");

  ck_assert (hash_insert ("yadda", "really?!"));
  ck_assert_str_eq (hash_find ("yadda"), "really?!");
}
END_TEST

START_TEST (C_hash_collisions)
{
  char *strings[] = { "foo", "bar", "zoo", "yadda", "help", "goo", "hi", "boo",
    "nine", "ten" };

  hash_init (100);

  for (int i = 0; i < 10; i++)
    ck_assert (hash_insert (strings[i], strings[(i + 1) % 10]));

  ck_assert (hash_insert ("bar", "a"));
  ck_assert (hash_insert ("help", "b"));
  ck_assert_str_eq (hash_find ("bar"), "a");
  ck_assert_str_eq (hash_find ("help"), "b");

  ck_assert (hash_remove ("bar"));
  ck_assert (hash_remove ("help"));
  ck_assert (hash_find ("bar") == NULL);
  ck_assert (hash_find ("help") == NULL);
  
  ck_assert (hash_insert ("help", "asldasdffkj"));
  ck_assert (hash_insert ("ten", "asldfkj"));
  ck_assert_str_eq (hash_find ("help"), "asldasdffkj");
  ck_assert_str_eq (hash_find ("ten"), "asldfkj");
}
END_TEST

void
public_tests (Suite *s)
{
  TCase *tc_public = tcase_create ("Public");
  tcase_add_test (tc_public, C_hash_insert);
  tcase_add_test (tc_public, C_hash_find_missing);
  tcase_add_test (tc_public, C_hash_remove);
  tcase_add_test (tc_public, C_hash_replace);
  tcase_add_test (tc_public, C_hash_collisions);
  suite_add_tcase (s, tc_public);
}
