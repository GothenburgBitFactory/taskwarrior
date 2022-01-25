#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "taskchampion.h"

// creating strings does not crash
static void test_string_creation(void) {
    TCString *s = tc_string_borrow("abcdef");
    tc_string_free(s);
}

// creating cloned strings does not crash
static void test_string_cloning(void) {
    char *abcdef = strdup("abcdef");
    TCString *s = tc_string_clone(abcdef);
    TEST_ASSERT_NOT_NULL(s);
    free(abcdef);

    TEST_ASSERT_EQUAL_STRING("abcdef", tc_string_content(s));
    tc_string_free(s);
}

// borrowed strings echo back their content
static void test_string_borrowed_strings_echo(void) {
    TCString *s = tc_string_borrow("abcdef");
    TEST_ASSERT_NOT_NULL(s);

    TEST_ASSERT_EQUAL_STRING("abcdef", tc_string_content(s));

    size_t len;
    const char *buf = tc_string_content_with_len(s, &len);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQUAL(6, len);
    TEST_ASSERT_EQUAL_MEMORY("abcdef", buf, len);

    tc_string_free(s);
}

// cloned strings echo back their content
static void test_string_cloned_strings_echo(void) {
    char *orig = strdup("abcdef");
    TCString *s = tc_string_clone(orig);
    TEST_ASSERT_NOT_NULL(s);
    free(orig);

    TEST_ASSERT_EQUAL_STRING("abcdef", tc_string_content(s));

    size_t len;
    const char *buf = tc_string_content_with_len(s, &len);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQUAL(6, len);
    TEST_ASSERT_EQUAL_MEMORY("abcdef", buf, len);

    tc_string_free(s);
}

// tc_string_content returns NULL for strings containing embedded NULs
static void test_string_content_null_for_embedded_nuls(void) {
    TCString *s = tc_string_clone_with_len("ab\0de", 5);
    TEST_ASSERT_NOT_NULL(s);

    TEST_ASSERT_NULL(tc_string_content(s));

    size_t len;
    const char *buf = tc_string_content_with_len(s, &len);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQUAL(5, len);
    TEST_ASSERT_EQUAL_MEMORY("ab\0de", buf, len);
    tc_string_free(s);
}

int string_tests(void) {
    UNITY_BEGIN();
    // each test case above should be named here, in order.
    RUN_TEST(test_string_creation);
    RUN_TEST(test_string_cloning);
    RUN_TEST(test_string_borrowed_strings_echo);
    RUN_TEST(test_string_cloned_strings_echo);
    RUN_TEST(test_string_content_null_for_embedded_nuls);
    return UNITY_END();
}
