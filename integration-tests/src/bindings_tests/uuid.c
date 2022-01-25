#include <string.h>
#include "unity.h"
#include "taskchampion.h"

// creating UUIDs does not crash
static void test_uuid_creation(void) {
    tc_uuid_new_v4();
    tc_uuid_nil();
}

// converting UUIDs from string works
static void test_uuid_conversion_to_string(void) {
    TEST_ASSERT_EQUAL(TC_UUID_STRING_BYTES, 36);

    TCUuid u2 = tc_uuid_nil();

    char u2str[TC_UUID_STRING_BYTES];
    tc_uuid_to_str(u2, u2str);
    TEST_ASSERT_EQUAL_MEMORY("00000000-0000-0000-0000-000000000000", u2str, TC_UUID_STRING_BYTES);
}

// converting invalid UUIDs from string fails as expected
static void test_uuid_invalid_string_fails(void) {
    TCUuid u;
    char ustr[36] = "not-a-valid-uuid";
    TEST_ASSERT_FALSE(tc_uuid_from_str(ustr, &u));
}

// converting invalid UTF-8 UUIDs from string fails as expected
static void test_uuid_bad_utf8(void) {
    TCUuid u;
    char ustr[36] = "\xf0\x28\x8c\xbc";
    TEST_ASSERT_FALSE(tc_uuid_from_str(ustr, &u));
}

int uuid_tests(void) {
    UNITY_BEGIN();
    // each test case above should be named here, in order.
    RUN_TEST(test_uuid_creation);
    RUN_TEST(test_uuid_conversion_to_string);
    RUN_TEST(test_uuid_invalid_string_fails);
    RUN_TEST(test_uuid_bad_utf8);
    return UNITY_END();
}
