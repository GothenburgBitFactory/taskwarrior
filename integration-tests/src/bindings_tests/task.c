#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "taskchampion.h"

// creating a task succeeds and the resulting task looks good
static void test_task_creation(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep));

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));

    TCString *desc = tc_task_get_description(task);
    TEST_ASSERT_NOT_NULL(desc);
    TEST_ASSERT_EQUAL_STRING("my task", tc_string_content(desc));
    tc_string_free(desc);

    tc_task_free(task);

    tc_replica_free(rep);
}

// freeing a mutable task works, marking it immutable
static void test_task_free_mutable_task(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep));

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));
    TCUuid uuid = tc_task_get_uuid(task);

    tc_task_to_mut(task, rep);
    TEST_ASSERT_EQUAL(TC_RESULT_TRUE, tc_task_set_status(task, TC_STATUS_DELETED));
    TEST_ASSERT_EQUAL(TC_STATUS_DELETED, tc_task_get_status(task));

    tc_task_free(task); // implicitly converts to immut

    task = tc_replica_get_task(rep, uuid);
    TEST_ASSERT_NOT_NULL(task);
    TEST_ASSERT_EQUAL(TC_STATUS_DELETED, tc_task_get_status(task));
    tc_task_free(task);

    tc_replica_free(rep);
}

// updating status on a task works
static void test_task_get_set_status(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep));

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));

    tc_task_to_mut(task, rep);
    TEST_ASSERT_EQUAL(TC_RESULT_TRUE, tc_task_set_status(task, TC_STATUS_DELETED));
    TEST_ASSERT_EQUAL(TC_STATUS_DELETED, tc_task_get_status(task)); // while mut
    tc_task_to_immut(task);
    TEST_ASSERT_EQUAL(TC_STATUS_DELETED, tc_task_get_status(task)); // while immut

    tc_task_free(task);

    tc_replica_free(rep);
}

// updating description on a task works
static void test_task_get_set_description(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep));

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    TCString *desc;

    tc_task_to_mut(task, rep);
    TEST_ASSERT_EQUAL(TC_RESULT_TRUE, tc_task_set_description(task, tc_string_borrow("updated")));

    TEST_ASSERT_TRUE(desc = tc_task_get_description(task));
    TEST_ASSERT_NOT_NULL(desc);
    TEST_ASSERT_EQUAL_STRING("updated", tc_string_content(desc));
    tc_string_free(desc);

    tc_task_to_immut(task);

    desc = tc_task_get_description(task);
    TEST_ASSERT_NOT_NULL(desc);
    TEST_ASSERT_EQUAL_STRING("updated", tc_string_content(desc));
    tc_string_free(desc);

    tc_task_free(task);

    tc_replica_free(rep);
}

// starting and stopping a task works, as seen by tc_task_is_active
static void test_task_start_stop_is_active(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep));

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    TEST_ASSERT_FALSE(tc_task_is_active(task));

    tc_task_to_mut(task, rep);

    TEST_ASSERT_FALSE(tc_task_is_active(task));
    TEST_ASSERT_EQUAL(TC_RESULT_TRUE, tc_task_start(task));
    TEST_ASSERT_TRUE(tc_task_is_active(task));
    TEST_ASSERT_EQUAL(TC_RESULT_TRUE, tc_task_stop(task));
    TEST_ASSERT_FALSE(tc_task_is_active(task));

    tc_task_free(task);
    tc_replica_free(rep);
}

// adding tags to a task works, and invalid tags are rejected
static void task_task_add_tag(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep));

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    tc_task_to_mut(task, rep);

    TEST_ASSERT_EQUAL(TC_RESULT_TRUE, tc_task_add_tag(task, tc_string_borrow("next")));

    // invalid - synthetic tag
    TEST_ASSERT_EQUAL(TC_RESULT_ERROR, tc_task_add_tag(task, tc_string_borrow("PENDING")));
    // invald - not a valid tag string
    TEST_ASSERT_EQUAL(TC_RESULT_ERROR, tc_task_add_tag(task, tc_string_borrow("my tag")));
    // invald - not utf-8
    TEST_ASSERT_EQUAL(TC_RESULT_ERROR, tc_task_add_tag(task, tc_string_borrow("\xf0\x28\x8c\x28")));

    // TODO: check error messages
    // TODO: test getting the tag

    tc_task_free(task);
    tc_replica_free(rep);
}

int task_tests(void) {
    UNITY_BEGIN();
    // each test case above should be named here, in order.
    RUN_TEST(test_task_creation);
    RUN_TEST(test_task_free_mutable_task);
    RUN_TEST(test_task_get_set_status);
    RUN_TEST(test_task_get_set_description);
    RUN_TEST(test_task_start_stop_is_active);
    RUN_TEST(task_task_add_tag);
    return UNITY_END();
}
