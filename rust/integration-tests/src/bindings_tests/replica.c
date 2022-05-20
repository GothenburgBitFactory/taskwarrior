#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "taskchampion.h"
#include "unity.h"

// creating an in-memory replica does not crash
static void test_replica_creation(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NOT_NULL(rep);
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);
    tc_replica_free(rep);
}

// creating an on-disk replica does not crash
static void test_replica_creation_disk(void) {
    TCReplica *rep = tc_replica_new_on_disk(tc_string_borrow("test-db"), NULL);
    TEST_ASSERT_NOT_NULL(rep);
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);
    tc_replica_free(rep);
}

// undo on an empty in-memory TCReplica does nothing
static void test_replica_undo_empty(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);
    int undone;
    int rv = tc_replica_undo(rep, &undone);
    TEST_ASSERT_EQUAL(TC_RESULT_OK, rv);
    TEST_ASSERT_EQUAL(0, undone);
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);
    tc_replica_free(rep);
}

// adding an undo point succeeds
static void test_replica_add_undo_point(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_replica_add_undo_point(rep, true));
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);
    tc_replica_free(rep);
}

// working set operations succeed
static void test_replica_working_set(void) {
    TCWorkingSet *ws;
    TCTask *task1, *task2, *task3;
    TCUuid uuid, uuid1, uuid2, uuid3;

    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_replica_rebuild_working_set(rep, true));
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    ws = tc_replica_working_set(rep);
    TEST_ASSERT_EQUAL(0, tc_working_set_len(ws));
    tc_working_set_free(ws);

    task1 = tc_replica_new_task(rep, TC_STATUS_PENDING, tc_string_borrow("task1"));
    TEST_ASSERT_NOT_NULL(task1);
    uuid1 = tc_task_get_uuid(task1);

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_replica_rebuild_working_set(rep, true));
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    task2 = tc_replica_new_task(rep, TC_STATUS_PENDING, tc_string_borrow("task2"));
    TEST_ASSERT_NOT_NULL(task2);
    uuid2 = tc_task_get_uuid(task2);

    task3 = tc_replica_new_task(rep, TC_STATUS_PENDING, tc_string_borrow("task3"));
    TEST_ASSERT_NOT_NULL(task3);
    uuid3 = tc_task_get_uuid(task3);

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_replica_rebuild_working_set(rep, false));
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    // finish task2 to leave a "hole"
    tc_task_to_mut(task2, rep);
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_done(task2));
    tc_task_to_immut(task2);

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_replica_rebuild_working_set(rep, false));
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    tc_task_free(task1);
    tc_task_free(task2);
    tc_task_free(task3);

    // working set should now be
    //  0 -> None
    //  1 -> uuid1
    //  2 -> None
    //  3 -> uuid3
    ws = tc_replica_working_set(rep);
    TEST_ASSERT_EQUAL(2, tc_working_set_len(ws));
    TEST_ASSERT_EQUAL(3, tc_working_set_largest_index(ws));

    TEST_ASSERT_FALSE(tc_working_set_by_index(ws, 0, &uuid));
    TEST_ASSERT_TRUE(tc_working_set_by_index(ws, 1, &uuid));
    TEST_ASSERT_EQUAL_MEMORY(uuid1.bytes, uuid.bytes, sizeof(uuid));
    TEST_ASSERT_FALSE(tc_working_set_by_index(ws, 2, &uuid));
    TEST_ASSERT_TRUE(tc_working_set_by_index(ws, 3, &uuid));
    TEST_ASSERT_EQUAL_MEMORY(uuid3.bytes, uuid.bytes, sizeof(uuid));

    TEST_ASSERT_EQUAL(1, tc_working_set_by_uuid(ws, uuid1));
    TEST_ASSERT_EQUAL(0, tc_working_set_by_uuid(ws, uuid2));
    TEST_ASSERT_EQUAL(3, tc_working_set_by_uuid(ws, uuid3));

    tc_working_set_free(ws);

    TEST_ASSERT_EQUAL(19, tc_replica_num_local_operations(rep));

    tc_replica_free(rep);
}

// When tc_replica_undo is passed NULL for undone_out, it still succeeds
static void test_replica_undo_empty_null_undone_out(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);
    int rv = tc_replica_undo(rep, NULL);
    TEST_ASSERT_EQUAL(TC_RESULT_OK, rv);
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);
    tc_replica_free(rep);
}

// creating a task succeeds and the resulting task looks good
static void test_replica_task_creation(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    TCUuid uuid = tc_task_get_uuid(task);
    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));

    TCString desc = tc_task_get_description(task);
    TEST_ASSERT_NOT_NULL(desc.ptr);
    TEST_ASSERT_EQUAL_STRING("my task", tc_string_content(&desc));
    tc_string_free(&desc);

    tc_task_free(task);

    // get the task again and verify it
    task = tc_replica_get_task(rep, uuid);
    TEST_ASSERT_NOT_NULL(task);
    TEST_ASSERT_EQUAL_MEMORY(uuid.bytes, tc_task_get_uuid(task).bytes, sizeof(uuid.bytes));
    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));

    tc_task_free(task);

    tc_replica_free(rep);
}

// When tc_replica_undo is passed NULL for undone_out, it still succeeds
static void test_replica_sync_local(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    mkdir("test-sync-server", 0755); // ignore error, if dir already exists

    TCString err;
    TCServer *server = tc_server_new_local(tc_string_borrow("test-sync-server"), &err);
    TEST_ASSERT_NOT_NULL(server);
    TEST_ASSERT_NULL(err.ptr);

    int rv = tc_replica_sync(rep, server, false);
    TEST_ASSERT_EQUAL(TC_RESULT_OK, rv);
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    tc_server_free(server);
    tc_replica_free(rep);

    // test error handling
    server = tc_server_new_local(tc_string_borrow("/no/such/directory"), &err);
    TEST_ASSERT_NULL(server);
    TEST_ASSERT_NOT_NULL(err.ptr);
    tc_string_free(&err);
}

// When tc_replica_undo is passed NULL for undone_out, it still succeeds
static void test_replica_remote_server(void) {
    TCString err;
    TCServer *server = tc_server_new_remote(
        tc_string_borrow("tc.freecinc.com"),
        tc_uuid_new_v4(),
        tc_string_borrow("\xf0\x28\x8c\x28"), // NOTE: not utf-8
        &err);
    TEST_ASSERT_NOT_NULL(server);
    TEST_ASSERT_NULL(err.ptr);

    // can't actually do anything with this server!

    tc_server_free(server);
}

// a replica with tasks in it returns an appropriate list of tasks and list of uuids
static void test_replica_all_tasks(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task1 = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("task1"));
    TEST_ASSERT_NOT_NULL(task1);
    TCUuid uuid1 = tc_task_get_uuid(task1);
    tc_task_free(task1);

    TCTask *task2 = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("task2"));
    TEST_ASSERT_NOT_NULL(task2);
    TCUuid uuid2 = tc_task_get_uuid(task2);
    tc_task_free(task2);

    {
        TCTaskList tasks = tc_replica_all_tasks(rep);
        TEST_ASSERT_NOT_NULL(tasks.items);
        TEST_ASSERT_EQUAL(2, tasks.len);

        bool seen1, seen2 = false;
        for (size_t i = 0; i < tasks.len; i++) {
            TCTask *task = tasks.items[i];
            TCString descr = tc_task_get_description(task);
            if (0 == strcmp(tc_string_content(&descr), "task1")) {
                seen1 = true;
            } else if (0 == strcmp(tc_string_content(&descr), "task2")) {
                seen2 = true;
            }
            tc_string_free(&descr);
        }
        TEST_ASSERT_TRUE(seen1);
        TEST_ASSERT_TRUE(seen2);

        tc_task_list_free(&tasks);
        TEST_ASSERT_NULL(tasks.items);
    }

    {
        TCUuidList uuids = tc_replica_all_task_uuids(rep);
        TEST_ASSERT_NOT_NULL(uuids.items);
        TEST_ASSERT_EQUAL(2, uuids.len);

        bool seen1, seen2 = false;
        for (size_t i = 0; i < uuids.len; i++) {
            TCUuid uuid = uuids.items[i];
            if (0 == memcmp(&uuid1, &uuid, sizeof(TCUuid))) {
                seen1 = true;
            } else if (0 == memcmp(&uuid2, &uuid, sizeof(TCUuid))) {
                seen2 = true;
            }
        }
        TEST_ASSERT_TRUE(seen1);
        TEST_ASSERT_TRUE(seen2);

        tc_uuid_list_free(&uuids);
        TEST_ASSERT_NULL(uuids.items);
    }

    tc_replica_free(rep);
}

// importing a task succeeds and the resulting task looks good
static void test_replica_task_import(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCUuid uuid;
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_uuid_from_str(tc_string_borrow("23cb25e0-5d1a-4932-8131-594ac6d3a843"), &uuid));
    TCTask *task = tc_replica_import_task_with_uuid(rep, uuid);
    TEST_ASSERT_NOT_NULL(task);

    TEST_ASSERT_EQUAL_MEMORY(uuid.bytes, tc_task_get_uuid(task).bytes, sizeof(uuid.bytes));
    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));

    TCString desc = tc_task_get_description(task);
    TEST_ASSERT_NOT_NULL(desc.ptr);
    TEST_ASSERT_EQUAL_STRING("", tc_string_content(&desc)); // default value
    tc_string_free(&desc);

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
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCUuid uuid;
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_uuid_from_str(tc_string_borrow("23cb25e0-5d1a-4932-8131-594ac6d3a843"), &uuid));
    TCTask *task = tc_replica_get_task(rep, uuid);
    TEST_ASSERT_NULL(task);
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    tc_replica_free(rep);
}

int replica_tests(void) {
    UNITY_BEGIN();
    // each test case above should be named here, in order.
    RUN_TEST(test_replica_creation);
    RUN_TEST(test_replica_creation_disk);
    RUN_TEST(test_replica_undo_empty);
    RUN_TEST(test_replica_add_undo_point);
    RUN_TEST(test_replica_working_set);
    RUN_TEST(test_replica_undo_empty_null_undone_out);
    RUN_TEST(test_replica_task_creation);
    RUN_TEST(test_replica_sync_local);
    RUN_TEST(test_replica_remote_server);
    RUN_TEST(test_replica_all_tasks);
    RUN_TEST(test_replica_task_import);
    RUN_TEST(test_replica_get_task_not_found);
    return UNITY_END();
}
