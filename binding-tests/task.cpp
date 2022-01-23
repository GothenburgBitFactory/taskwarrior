#include <string.h>
#include "doctest.h"
#include "taskchampion.h"

TEST_CASE("creating a Task does not crash") {
    TCReplica *rep = tc_replica_new(NULL);
    CHECK(tc_replica_error(rep) == NULL);

    TCTask *task = tc_replica_new_task(
            rep,
            TC_STATUS_PENDING,
            tc_string_new("my task"));
    REQUIRE(task != NULL);

    CHECK(tc_task_get_status(task) == TC_STATUS_PENDING);

    TCString *desc = tc_task_get_description(task);
    REQUIRE(desc != NULL);
    CHECK(strcmp(tc_string_content(desc), "my task") == 0);
    tc_string_free(desc);

    tc_task_free(task);

    tc_replica_free(rep);
}

TEST_CASE("undo on an empty in-memory TCReplica does nothing") {
    TCReplica *rep = tc_replica_new(NULL);
    CHECK(tc_replica_error(rep) == NULL);
    int rv = tc_replica_undo(rep);
    CHECK(rv == 0);
    CHECK(tc_replica_error(rep) == NULL);
    tc_replica_free(rep);
}

