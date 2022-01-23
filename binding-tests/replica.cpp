#include <string.h>
#include "doctest.h"
#include "taskchampion.h"

TEST_CASE("creating an in-memory TCReplica does not crash") {
    TCReplica *rep = tc_replica_new(NULL);
    CHECK(tc_replica_error(rep) == NULL);
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
