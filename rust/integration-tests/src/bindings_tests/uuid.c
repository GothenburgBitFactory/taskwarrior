#include <string.h>
#include "unity.h"
#include "taskchampion.h"

// creating UUIDs does not crash
static void test_uuid_creation(void) {
    tc_uuid_new_v4();
    tc_uuid_nil();
}

// converting UUIDs to a buf works
static void test_uuid_to_buf(void) {
    TEST_ASSERT_EQUAL(TC_UUID_STRING_BYTES, 36);

    TCUuid u2 = tc_uuid_nil();

    char u2str[TC_UUID_STRING_BYTES];
    tc_uuid_to_buf(u2, u2str);
    TEST_ASSERT_EQUAL_MEMORY("00000000-0000-0000-0000-000000000000", u2str, TC_UUID_STRING_BYTES);
}

// converting UUIDs to a buf works
static void test_uuid_to_str(void) {
    TCUuid u = tc_uuid_nil();
    TCString s = tc_uuid_to_str(u);
    TEST_ASSERT_EQUAL_STRING(
        "00000000-0000-0000-0000-000000000000",
        tc_string_content(&s));
    tc_string_free(&s);
}

// converting valid UUIDs from string works
static void test_uuid_valid_from_str(void) {
    TCUuid u;
    char *ustr = "23cb25e0-5d1a-4932-8131-594ac6d3a843";
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_uuid_from_str(tc_string_borrow(ustr), &u));
    TEST_ASSERT_EQUAL(0x23, u.bytes[0]);
    TEST_ASSERT_EQUAL(0x43, u.bytes[15]);
}

// converting invalid UUIDs from string fails as expected
static void test_uuid_invalid_string_fails(void) {
    TCUuid u;
    char *ustr = "not-a-valid-uuid";
    TEST_ASSERT_EQUAL(TC_RESULT_ERROR, tc_uuid_from_str(tc_string_borrow(ustr), &u));
}

// converting invalid UTF-8 UUIDs from string fails as expected
static void test_uuid_bad_utf8(void) {
    TCUuid u;
    char *ustr = "\xf0\x28\x8c\xbc";
    TEST_ASSERT_EQUAL(TC_RESULT_ERROR, tc_uuid_from_str(tc_string_borrow(ustr), &u));
}

// converting a string with embedded NUL fails as expected
static void test_uuid_embedded_nul(void) {
    TCUuid u;
    TEST_ASSERT_EQUAL(TC_RESULT_ERROR, tc_uuid_from_str(tc_string_clone_with_len("ab\0de", 5), &u));
}

int uuid_tests(void) {
    UNITY_BEGIN();
    // each test case above should be named here, in order.
    RUN_TEST(test_uuid_creation);
    RUN_TEST(test_uuid_valid_from_str);
    RUN_TEST(test_uuid_to_buf);
    RUN_TEST(test_uuid_to_str);
    RUN_TEST(test_uuid_invalid_string_fails);
    RUN_TEST(test_uuid_bad_utf8);
    RUN_TEST(test_uuid_embedded_nul);
    return UNITY_END();
}
