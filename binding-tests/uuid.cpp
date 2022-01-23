#include <string.h>
#include "doctest.h"
#include "taskchampion.h"

TEST_CASE("creating UUIDs does not crash") {
    Uuid u1 = tc_uuid_new_v4();
    Uuid u2 = tc_uuid_nil();
}

TEST_CASE("converting UUIDs to string works") {
    Uuid u2 = tc_uuid_nil();
    REQUIRE(TC_UUID_STRING_BYTES == 36);

    char u2str[TC_UUID_STRING_BYTES];
    tc_uuid_to_str(u2, u2str);
    CHECK(strncmp(u2str, "00000000-0000-0000-0000-000000000000", TC_UUID_STRING_BYTES) == 0);
}

TEST_CASE("converting UUIDs from string works") {
    Uuid u;
    char ustr[TC_UUID_STRING_BYTES] = "fdc314b7-f938-4845-b8d1-95716e4eb762";
    CHECK(tc_uuid_from_str(ustr, &u));
    CHECK(u._0[0] == 0xfd);
    // .. if these two are correct, probably it worked :)
    CHECK(u._0[15] == 0x62);
}

TEST_CASE("converting invalid UUIDs from string fails as expected") {
    Uuid u;
    char ustr[TC_UUID_STRING_BYTES] = "not-a-valid-uuid";
    CHECK(!tc_uuid_from_str(ustr, &u));
}

TEST_CASE("converting invalid UTF-8 UUIDs from string fails as expected") {
    Uuid u;
    char ustr[TC_UUID_STRING_BYTES] = "\xf0\x28\x8c\xbc";
    CHECK(!tc_uuid_from_str(ustr, &u));
}
