#include "doctest.h"
#include "taskchampion.h"

TEST_CASE("creating a UUID") {
    StoragePtr *storage = storage_new_in_memory();
    storage_free(storage);
}
