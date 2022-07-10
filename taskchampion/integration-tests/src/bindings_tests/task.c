#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "taskchampion.h"

// creating a task succeeds and the resulting task looks good
static void test_task_creation(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));

    TCString desc = tc_task_get_description(task);
    TEST_ASSERT_NOT_NULL(desc.ptr);
    TEST_ASSERT_EQUAL_STRING("my task", tc_string_content(&desc));
    tc_string_free(&desc);

    tc_task_free(task);

    tc_replica_free(rep);
}

// freeing a mutable task works, marking it immutable
static void test_task_free_mutable_task(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));
    TCUuid uuid = tc_task_get_uuid(task);

    tc_task_to_mut(task, rep);
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_set_status(task, TC_STATUS_DELETED));
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
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));

    tc_task_to_mut(task, rep);
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_set_status(task, TC_STATUS_DELETED));
    TEST_ASSERT_EQUAL(TC_STATUS_DELETED, tc_task_get_status(task)); // while mut
    tc_task_to_immut(task);
    TEST_ASSERT_EQUAL(TC_STATUS_DELETED, tc_task_get_status(task)); // while immut

    tc_task_free(task);

    tc_replica_free(rep);
}

// updating description on a task works
static void test_task_get_set_description(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    TCString desc;

    tc_task_to_mut(task, rep);
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_set_description(task, tc_string_borrow("updated")));

    desc = tc_task_get_description(task);
    TEST_ASSERT_NOT_NULL(desc.ptr);
    TEST_ASSERT_EQUAL_STRING("updated", tc_string_content(&desc));
    tc_string_free(&desc);

    tc_task_to_immut(task);

    desc = tc_task_get_description(task);
    TEST_ASSERT_NOT_NULL(desc.ptr);
    TEST_ASSERT_EQUAL_STRING("updated", tc_string_content(&desc));
    tc_string_free(&desc);

    tc_task_free(task);

    tc_replica_free(rep);
}

// updating entry on a task works
static void test_task_get_set_entry(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    // creation of a task sets entry to current time
    TEST_ASSERT_NOT_EQUAL(0, tc_task_get_entry(task));

    tc_task_to_mut(task, rep);

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_set_entry(task, 1643679997));
    TEST_ASSERT_EQUAL(1643679997, tc_task_get_entry(task));

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_set_entry(task, 0));
    TEST_ASSERT_EQUAL(0, tc_task_get_entry(task));

    tc_task_free(task);

    tc_replica_free(rep);
}

// updating wait on a task works
static void test_task_get_set_wait_and_is_waiting(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    // wait is not set on creation
    TEST_ASSERT_EQUAL(0, tc_task_get_wait(task));
    TEST_ASSERT_FALSE(tc_task_is_waiting(task));

    tc_task_to_mut(task, rep);

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_set_wait(task, 3643679997)); // 2085
    TEST_ASSERT_EQUAL(3643679997, tc_task_get_wait(task));
    TEST_ASSERT_TRUE(tc_task_is_waiting(task));

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_set_wait(task, 643679997)); // THE PAST!
    TEST_ASSERT_EQUAL(643679997, tc_task_get_wait(task));
    TEST_ASSERT_FALSE(tc_task_is_waiting(task));

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_set_wait(task, 0));
    TEST_ASSERT_EQUAL(0, tc_task_get_wait(task));
    TEST_ASSERT_FALSE(tc_task_is_waiting(task));

    tc_task_free(task);

    tc_replica_free(rep);
}

// updating modified on a task works
static void test_task_get_set_modified(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    // creation of a task sets modified to current time
    TEST_ASSERT_NOT_EQUAL(0, tc_task_get_modified(task));

    tc_task_to_mut(task, rep);

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_set_modified(task, 1643679997));
    TEST_ASSERT_EQUAL(1643679997, tc_task_get_modified(task));

    // zero is not allowed
    TEST_ASSERT_EQUAL(TC_RESULT_ERROR, tc_task_set_modified(task, 0));

    tc_task_free(task);

    tc_replica_free(rep);
}

// starting and stopping a task works, as seen by tc_task_is_active
static void test_task_start_stop_is_active(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    TEST_ASSERT_FALSE(tc_task_is_active(task));

    tc_task_to_mut(task, rep);

    TEST_ASSERT_FALSE(tc_task_is_active(task));
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_start(task));
    TEST_ASSERT_TRUE(tc_task_is_active(task));
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_stop(task));
    TEST_ASSERT_FALSE(tc_task_is_active(task));

    tc_task_free(task);
    tc_replica_free(rep);
}

// tc_task_done and delete work and set the status
static void test_task_done_and_delete(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    tc_task_to_mut(task, rep);

    TEST_ASSERT_EQUAL(TC_STATUS_PENDING, tc_task_get_status(task));
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_done(task));
    TEST_ASSERT_EQUAL(TC_STATUS_COMPLETED, tc_task_get_status(task));
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_delete(task));
    TEST_ASSERT_EQUAL(TC_STATUS_DELETED, tc_task_get_status(task));

    tc_task_free(task);
    tc_replica_free(rep);
}

// adding and removing tags to a task works, and invalid tags are rejected
static void test_task_add_remove_has_tag(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    tc_task_to_mut(task, rep);

    TEST_ASSERT_FALSE(tc_task_has_tag(task, tc_string_borrow("next")));

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_add_tag(task, tc_string_borrow("next")));
    TEST_ASSERT_NULL(tc_task_error(task).ptr);

    TEST_ASSERT_TRUE(tc_task_has_tag(task, tc_string_borrow("next")));

    // invalid - synthetic tag
    TEST_ASSERT_EQUAL(TC_RESULT_ERROR, tc_task_add_tag(task, tc_string_borrow("PENDING")));
    TCString err = tc_task_error(task);
    TEST_ASSERT_NOT_NULL(err.ptr);
    tc_string_free(&err);

    // invald - not a valid tag string
    TEST_ASSERT_EQUAL(TC_RESULT_ERROR, tc_task_add_tag(task, tc_string_borrow("my tag")));
    err = tc_task_error(task);
    TEST_ASSERT_NOT_NULL(err.ptr);
    tc_string_free(&err);

    // invald - not utf-8
    TEST_ASSERT_EQUAL(TC_RESULT_ERROR, tc_task_add_tag(task, tc_string_borrow("\xf0\x28\x8c\x28")));
    err = tc_task_error(task);
    TEST_ASSERT_NOT_NULL(err.ptr);
    tc_string_free(&err);

    TEST_ASSERT_TRUE(tc_task_has_tag(task, tc_string_borrow("next")));

    // remove the tag
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_remove_tag(task, tc_string_borrow("next")));
    TEST_ASSERT_NULL(tc_task_error(task).ptr);

    TEST_ASSERT_FALSE(tc_task_has_tag(task, tc_string_borrow("next")));

    tc_task_free(task);
    tc_replica_free(rep);
}

// get_tags returns the list of tags
static void test_task_get_tags(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    tc_task_to_mut(task, rep);

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_add_tag(task, tc_string_borrow("next")));

    TCStringList tags = tc_task_get_tags(task);

    int found_pending = false, found_next = false;
    for (size_t i = 0; i < tags.len; i++) {
        if (strcmp("PENDING", tc_string_content(&tags.items[i])) == 0) {
            found_pending = true;
        }
        if (strcmp("next", tc_string_content(&tags.items[i])) == 0) {
            found_next = true;
        }
    }
    TEST_ASSERT_TRUE(found_pending);
    TEST_ASSERT_TRUE(found_next);

    tc_string_list_free(&tags);
    TEST_ASSERT_NULL(tags.items);

    tc_task_free(task);
    tc_replica_free(rep);
}

// annotation manipulation (add, remove, list, free)
static void test_task_annotations(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    TCAnnotationList anns = tc_task_get_annotations(task);
    TEST_ASSERT_EQUAL(0, anns.len);
    TEST_ASSERT_NOT_NULL(anns.items);
    tc_annotation_list_free(&anns);

    tc_task_to_mut(task, rep);

    TCAnnotation ann;

    ann.entry = 1644623411;
    ann.description = tc_string_borrow("ann1");
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_add_annotation(task, &ann));
    TEST_ASSERT_NULL(ann.description.ptr);

    ann.entry = 1644623422;
    ann.description = tc_string_borrow("ann2");
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_add_annotation(task, &ann));
    TEST_ASSERT_NULL(ann.description.ptr);

    anns = tc_task_get_annotations(task);

    int found1 = false, found2 = false;
    for (size_t i = 0; i < anns.len; i++) {
        if (0 == strcmp("ann1", tc_string_content(&anns.items[i].description))) {
            TEST_ASSERT_EQUAL(anns.items[i].entry, 1644623411);
            found1 = true;
        }
        if (0 == strcmp("ann2", tc_string_content(&anns.items[i].description))) {
            TEST_ASSERT_EQUAL(anns.items[i].entry, 1644623422);
            found2 = true;
        }
    }
    TEST_ASSERT_TRUE(found1);
    TEST_ASSERT_TRUE(found2);

    tc_annotation_list_free(&anns);
    TEST_ASSERT_NULL(anns.items);

    tc_task_free(task);
    tc_replica_free(rep);
}

// UDA manipulation (add, remove, list, free)
static void test_task_udas(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    tc_task_to_mut(task, rep);

    TCString value;
    TCUdaList udas;

    TEST_ASSERT_NULL(tc_task_get_uda(task, tc_string_borrow("ns"), tc_string_borrow("u1")).ptr);
    TEST_ASSERT_NULL(tc_task_get_legacy_uda(task, tc_string_borrow("leg1")).ptr);

    udas = tc_task_get_udas(task);
    TEST_ASSERT_NOT_NULL(udas.items);
    TEST_ASSERT_EQUAL(0, udas.len);
    tc_uda_list_free(&udas);

    udas = tc_task_get_legacy_udas(task);
    TEST_ASSERT_NOT_NULL(udas.items);
    TEST_ASSERT_EQUAL(0, udas.len);
    tc_uda_list_free(&udas);

    TEST_ASSERT_EQUAL(TC_RESULT_OK,
            tc_task_set_uda(task,
                tc_string_borrow("ns"),
                tc_string_borrow("u1"),
                tc_string_borrow("vvv")));

    value = tc_task_get_uda(task, tc_string_borrow("ns"), tc_string_borrow("u1"));
    TEST_ASSERT_NOT_NULL(value.ptr);
    TEST_ASSERT_EQUAL_STRING("vvv", tc_string_content(&value));
    tc_string_free(&value);
    TEST_ASSERT_NULL(tc_task_get_legacy_uda(task, tc_string_borrow("leg1")).ptr);

    udas = tc_task_get_udas(task);
    TEST_ASSERT_NOT_NULL(udas.items);
    TEST_ASSERT_EQUAL(1, udas.len);
    TEST_ASSERT_EQUAL_STRING("ns", tc_string_content(&udas.items[0].ns));
    TEST_ASSERT_EQUAL_STRING("u1", tc_string_content(&udas.items[0].key));
    TEST_ASSERT_EQUAL_STRING("vvv", tc_string_content(&udas.items[0].value));
    tc_uda_list_free(&udas);

    udas = tc_task_get_legacy_udas(task);
    TEST_ASSERT_NOT_NULL(udas.items);
    TEST_ASSERT_EQUAL(1, udas.len);
    TEST_ASSERT_NULL(udas.items[0].ns.ptr);
    TEST_ASSERT_EQUAL_STRING("ns.u1", tc_string_content(&udas.items[0].key));
    TEST_ASSERT_EQUAL_STRING("vvv", tc_string_content(&udas.items[0].value));
    tc_uda_list_free(&udas);

    TEST_ASSERT_EQUAL(TC_RESULT_OK,
            tc_task_set_legacy_uda(task,
                tc_string_borrow("leg1"),
                tc_string_borrow("legv")));

    value = tc_task_get_uda(task, tc_string_borrow("ns"), tc_string_borrow("u1"));
    TEST_ASSERT_NOT_NULL(value.ptr);
    TEST_ASSERT_EQUAL_STRING("vvv", tc_string_content(&value));
    tc_string_free(&value);

    value = tc_task_get_legacy_uda(task, tc_string_borrow("leg1"));
    TEST_ASSERT_NOT_NULL(value.ptr);
    TEST_ASSERT_EQUAL_STRING("legv", tc_string_content(&value));
    tc_string_free(&value);

    udas = tc_task_get_udas(task);
    TEST_ASSERT_NOT_NULL(udas.items);
    TEST_ASSERT_EQUAL(2, udas.len);
    tc_uda_list_free(&udas);

    udas = tc_task_get_legacy_udas(task);
    TEST_ASSERT_NOT_NULL(udas.items);
    TEST_ASSERT_EQUAL(2, udas.len);
    tc_uda_list_free(&udas);

    TEST_ASSERT_EQUAL(TC_RESULT_OK,
            tc_task_remove_uda(task,
                tc_string_borrow("ns"),
                tc_string_borrow("u1")));

    TEST_ASSERT_NULL(tc_task_get_uda(task, tc_string_borrow("ns"), tc_string_borrow("u1")).ptr);

    TEST_ASSERT_EQUAL(TC_RESULT_OK,
            tc_task_remove_uda(task,
                tc_string_borrow("ns"),
                tc_string_borrow("u1")));

    udas = tc_task_get_udas(task);
    TEST_ASSERT_NOT_NULL(udas.items);
    TEST_ASSERT_EQUAL(1, udas.len);
    TEST_ASSERT_EQUAL_STRING("", tc_string_content(&udas.items[0].ns));
    TEST_ASSERT_EQUAL_STRING("leg1", tc_string_content(&udas.items[0].key));
    TEST_ASSERT_EQUAL_STRING("legv", tc_string_content(&udas.items[0].value));
    tc_uda_list_free(&udas);

    udas = tc_task_get_legacy_udas(task);
    TEST_ASSERT_NOT_NULL(udas.items);
    TEST_ASSERT_EQUAL(1, udas.len);
    TEST_ASSERT_NULL(udas.items[0].ns.ptr);
    TEST_ASSERT_EQUAL_STRING("leg1", tc_string_content(&udas.items[0].key));
    TEST_ASSERT_EQUAL_STRING("legv", tc_string_content(&udas.items[0].value));
    tc_uda_list_free(&udas);

    TEST_ASSERT_EQUAL(TC_RESULT_OK,
            tc_task_remove_legacy_uda(task,
                tc_string_borrow("leg1")));

    TEST_ASSERT_NULL(tc_task_get_legacy_uda(task, tc_string_borrow("leg1")).ptr);

    TEST_ASSERT_EQUAL(TC_RESULT_OK,
            tc_task_remove_legacy_uda(task,
                tc_string_borrow("leg1")));

    udas = tc_task_get_udas(task);
    TEST_ASSERT_NOT_NULL(udas.items);
    TEST_ASSERT_EQUAL(0, udas.len);
    tc_uda_list_free(&udas);

    udas = tc_task_get_legacy_udas(task);
    TEST_ASSERT_NOT_NULL(udas.items);
    TEST_ASSERT_EQUAL(0, udas.len);
    tc_uda_list_free(&udas);

    tc_task_free(task);
    tc_replica_free(rep);
}

// dependency manipulation
static void test_task_dependencies(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task1 = tc_replica_new_task(rep, TC_STATUS_PENDING, tc_string_borrow("task 1"));
    TEST_ASSERT_NOT_NULL(task1);
    TCTask *task2 = tc_replica_new_task(rep, TC_STATUS_PENDING, tc_string_borrow("task 2"));
    TEST_ASSERT_NOT_NULL(task2);

    TCUuidList deps;

    deps = tc_task_get_dependencies(task1);
    TEST_ASSERT_EQUAL(0, deps.len);
    tc_uuid_list_free(&deps);

    tc_task_to_mut(task1, rep);
    TEST_ASSERT_EQUAL(TC_RESULT_OK,
            tc_task_add_dependency(task1, tc_task_get_uuid(task2)));

    deps = tc_task_get_dependencies(task1);
    TEST_ASSERT_EQUAL(1, deps.len);
    TEST_ASSERT_EQUAL_MEMORY(tc_task_get_uuid(task2).bytes, deps.items[0].bytes, 16);
    tc_uuid_list_free(&deps);

    TEST_ASSERT_EQUAL(TC_RESULT_OK,
            tc_task_remove_dependency(task1, tc_task_get_uuid(task2)));

    deps = tc_task_get_dependencies(task1);
    TEST_ASSERT_EQUAL(0, deps.len);
    tc_uuid_list_free(&deps);

    tc_task_free(task1);
    tc_task_free(task2);
    tc_replica_free(rep);
}

static void tckvlist_assert_key(TCKVList *list, char *key, char *value) {
    TEST_ASSERT_NOT_NULL(list);
    for (size_t i = 0; i < list->len; i++) {
        if (0 == strcmp(tc_string_content(&list->items[i].key), key)) {
            TEST_ASSERT_EQUAL_STRING(value, tc_string_content(&list->items[i].value));
            return;
        }
    }
    TEST_FAIL_MESSAGE("key not found");
}

// get_tags returns the list of tags
static void test_task_taskmap(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task = tc_replica_new_task(rep, TC_STATUS_PENDING, tc_string_borrow("my task"));
    TEST_ASSERT_NOT_NULL(task);

    tc_task_to_mut(task, rep);

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_add_tag(task, tc_string_borrow("next")));

    TCAnnotation ann;
    ann.entry = 1644623411;
    ann.description = tc_string_borrow("ann1");
    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_add_annotation(task, &ann));

    TEST_ASSERT_EQUAL(TC_RESULT_OK, tc_task_set_wait(task, 3643679997)); // 2085

    TCKVList taskmap = tc_task_get_taskmap(task);
    tckvlist_assert_key(&taskmap, "annotation_1644623411", "ann1");
    tckvlist_assert_key(&taskmap, "tag_next", "");
    tckvlist_assert_key(&taskmap, "status", "pending");
    tckvlist_assert_key(&taskmap, "description", "my task");
    tc_kv_list_free(&taskmap);

    tc_task_free(task);
    tc_replica_free(rep);
}

// taking from a task list behaves correctly
static void test_task_list_take(void) {
    TCReplica *rep = tc_replica_new_in_memory();
    TEST_ASSERT_NULL(tc_replica_error(rep).ptr);

    TCTask *task1 = tc_replica_new_task(rep, TC_STATUS_PENDING, tc_string_borrow("t"));
    TEST_ASSERT_NOT_NULL(task1);

    TCTask *task2 = tc_replica_new_task(rep, TC_STATUS_PENDING, tc_string_borrow("t"));
    TEST_ASSERT_NOT_NULL(task2);
    tc_task_free(task2);

    TCString desc;
    TCTaskList tasks = tc_replica_all_tasks(rep);
    TEST_ASSERT_NOT_NULL(tasks.items);
    TEST_ASSERT_EQUAL(2, tasks.len);

    task1 = tc_task_list_take(&tasks, 5); // out of bounds
    TEST_ASSERT_NULL(task1);

    task1 = tc_task_list_take(&tasks, 0);
    TEST_ASSERT_NOT_NULL(task1);
    desc = tc_task_get_description(task1);
    TEST_ASSERT_EQUAL_STRING("t", tc_string_content(&desc));
    tc_string_free(&desc);

    task2 = tc_task_list_take(&tasks, 1);
    TEST_ASSERT_NOT_NULL(task2);
    desc = tc_task_get_description(task2);
    TEST_ASSERT_EQUAL_STRING("t", tc_string_content(&desc));
    tc_string_free(&desc);

    tc_task_free(task1);
    tc_task_free(task2);

    task1 = tc_task_list_take(&tasks, 0); // already taken
    TEST_ASSERT_NULL(task1);

    task1 = tc_task_list_take(&tasks, 5); // out of bounds
    TEST_ASSERT_NULL(task1);

    tc_task_list_free(&tasks);
    TEST_ASSERT_NULL(tasks.items);

    tc_replica_free(rep);
}

int task_tests(void) {
    UNITY_BEGIN();
    // each test case above should be named here, in order.
    RUN_TEST(test_task_creation);
    RUN_TEST(test_task_free_mutable_task);
    RUN_TEST(test_task_get_set_status);
    RUN_TEST(test_task_get_set_description);
    RUN_TEST(test_task_get_set_entry);
    RUN_TEST(test_task_get_set_modified);
    RUN_TEST(test_task_get_set_wait_and_is_waiting);
    RUN_TEST(test_task_start_stop_is_active);
    RUN_TEST(test_task_done_and_delete);
    RUN_TEST(test_task_add_remove_has_tag);
    RUN_TEST(test_task_get_tags);
    RUN_TEST(test_task_annotations);
    RUN_TEST(test_task_udas);
    RUN_TEST(test_task_dependencies);
    RUN_TEST(test_task_taskmap);
    RUN_TEST(test_task_list_take);
    return UNITY_END();
}
