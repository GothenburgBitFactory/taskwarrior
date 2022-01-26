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

// creating a task succeeds and the resulting task looks good
static void test_replica_task_creation(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep));

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    TCUuid uuid = tc_task_get_uuid(task);
    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));

    TCString *desc = tc_task_get_description(task);
    TEST_ASSERT_NOT_NULL(desc);
    TEST_ASSERT_EQUAL_STRING("my task", tc_string_content(desc));
    tc_string_free(desc);

    tc_task_free(task);

    // get the task again and verify it
    task = tc_replica_get_task(rep, uuid);
    TEST_ASSERT_NOT_NULL(task);
    TEST_ASSERT_EQUAL_MEMORY(uuid.bytes, tc_task_get_uuid(task).bytes, sizeof(uuid.bytes));
    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));

    tc_task_free(task);

    tc_replica_free(rep);
}

// importing a task succeeds and the resulting task looks good
static void test_replica_task_import(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep));

    TCUuid uuid;
    TEST_ASSERT_TRUE(tc_uuid_from_str(tc_string_borrow("23cb25e0-5d1a-4932-8131-594ac6d3a843"), &uuid));
    TCTask *task = tc_replica_import_task_with_uuid(rep, uuid);
    TEST_ASSERT_NOT_NULL(task);

    TEST_ASSERT_EQUAL_MEMORY(uuid.bytes, tc_task_get_uuid(task).bytes, sizeof(uuid.bytes));
    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));

    TCString *desc = tc_task_get_description(task);
    TEST_ASSERT_NOT_NULL(desc);
    TEST_ASSERT_EQUAL_STRING("", tc_string_content(desc)); // default value
    tc_string_free(desc);

    tc_task_free(task);

    // get the task again and verify it
    task = tc_replica_get_task(rep, uuid);
    TEST_ASSERT_NOT_NULL(task);
    TEST_ASSERT_EQUAL_MEMORY(uuid.bytes, tc_task_get_uuid(task).bytes, sizeof(uuid.bytes));
    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));

    tc_task_free(task);

    tc_replica_free(rep);
}

// importing a task succeeds and the resulting task looks good
static void test_replica_get_task_not_found(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep));

    TCUuid uuid;
    TEST_ASSERT_TRUE(tc_uuid_from_str(tc_string_borrow("23cb25e0-5d1a-4932-8131-594ac6d3a843"), &uuid));
    TCTask *task = tc_replica_get_task(rep, uuid);
    TEST_ASSERT_NULL(task);
    TEST_ASSERT_NULL(tc_replica_error(rep));
}

int replica_tests(void) {
    UNITY_BEGIN();
    // each test case above should be named here, in order.
    RUN_TEST(test_replica_creation);
    RUN_TEST(test_replica_creation_disk);
    RUN_TEST(test_replica_undo_empty);
    RUN_TEST(test_replica_task_creation);
    RUN_TEST(test_replica_task_import);
    RUN_TEST(test_replica_get_task_not_found);
    return UNITY_END();
}
