#include <string.h>
#include "doctest.h"
#include "taskchampion.h"

TEST_CASE("creating an in-memory Replica does not crash") {
    Replica *rep = tc_replica_new(NULL);
    CHECK(tc_replica_error(rep) == NULL);
    uhoh(rep);
    REQUIRE(tc_replica_error(rep) != NULL);
    CHECK(strcmp(tc_replica_error(rep), "uhoh!") == 0);
    tc_replica_free(rep);
}
