#ifndef TEST_BASE_H
#define TEST_BASE_H

void
test_register(const char *group, const char *name, int (*fn)(void));


#define TEST(test_id) \
  static int schess_test_## test_id(void); \
  __attribute__((constructor)) static void \
  schess_testregister_## test_id(void) \
{ \
  test_register(__FILE__, #test_id, schess_test_## test_id); \
} \
static int schess_test_## test_id(void)



#endif // TEST_BASE_H
