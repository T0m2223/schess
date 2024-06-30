#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <test/base.h>



typedef struct test_ll
{
  const char *name;
  int (*fn)(void);
  struct test_ll *next;
} test_ll;


typedef struct group_ll
{
  const char *name;
  test_ll *tests;
  size_t count;
  struct group_ll *next;
} group_ll;


static group_ll *group_head;


void
test_ll_append_test(test_ll **head, const char *name, int (*fn)(void))
{
  test_ll *new_test, *current;

  new_test = malloc(sizeof(test_ll));
  new_test->name = name;
  new_test->fn = fn;
  new_test->next = NULL;

  if (!*head)
  {
    *head = new_test;
    return;
  }
  for (current = *head; current->next; current = current->next);
  current->next = new_test;
}

void
group_append_test(const char *group, const char *name, int (*fn)(void))
{
  group_ll *current, *last;

  if (!group_head)
  {
    group_head = malloc(sizeof(group_ll));
    group_head->name = group;
    group_head->next = NULL;
    group_head->tests = NULL;
    group_head->count = 1;
    test_ll_append_test(&group_head->tests, name, fn);
    return;
  }

  for (current = group_head; current; current = current->next)
  {
    if (!strcmp(current->name, group))
    {
      test_ll_append_test(&current->tests, name, fn);
      ++current->count;
      return;
    }
    last = current;
  }

  last->next = malloc(sizeof(group_ll));
  last = current->next;
  last->name = group;
  last->next = NULL;
  last->tests = NULL;
  last->count = 1;
  test_ll_append_test(&group_head->tests, name, fn);
}

void
test_register(const char *group, const char *name, int (*fn)(void))
{
  group_append_test(group, name, fn);
}


int
main(void)
{
  group_ll *current_group;
  test_ll *current_test;

  size_t num_tests, failures;
  size_t ran;
  int res;

  for (current_group = group_head; current_group; current_group = current_group->next)
  {
    ran = num_tests = failures = 0;
    printf("Running tests from %s:\n", current_group->name);
    for (current_test = current_group->tests; current_test; current_test = current_test->next, ++ran)
    {
      printf("=> Progress: (%zu/%zu)", ran, current_group->count);
      fflush(stdout);
      ++num_tests;
      res = current_test->fn();
      printf("\r");
      if (!res) continue;

      ++failures;
      printf("  => '%s' failed: %d\n", current_test->name, res);
    }

    printf("Test group %s: %zu test run; %zu failed.\n", current_group->name, num_tests, failures);
  }

  return EXIT_SUCCESS;
}
