#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "taskchampion.h"

// creating an in-memory replica does not crash
static void test_replica_creation(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NOT_NULL(rep);
    TEST_ASSERT_NULL(tc_replica_error(rep));
    tc_replica_free(rep);
}

// creating an on-disk replica does not crash
static void test_replica_creation_disk(void) {
    TCReplica *rep = tc_replica_new_on_disk(tc_string_borrow("test-db"), NULL);
    TEST_ASSERT_NOT_NULL(rep);
    TEST_ASSERT_NULL(tc_replica_error(rep));
    tc_replica_free(rep);
}

// undo on an empty in-memory TCReplica does nothing
static void test_replica_undo_empty(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep));
    int rv = tc_replica_undo(rep);
    TEST_ASSERT_EQUAL(TC_RESULT_FALSE, rv);
    TEST_ASSERT_NULL(tc_replica_error(rep));
    tc_replica_free(rep);
}

int replica_tests(void) {
    UNITY_BEGIN();
    // each test case above should be named here, in order.
    RUN_TEST(test_replica_creation);
    RUN_TEST(test_replica_creation_disk);
    RUN_TEST(test_replica_undo_empty);
    return UNITY_END();
}
