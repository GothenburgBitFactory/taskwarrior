#include <string.h>
#include "doctest.h"
#include "taskchampion.h"

TEST_CASE("creating borrowed strings does not crash") {
    TCString *s = tc_string_new("abcdef");
    tc_string_free(s);
}

TEST_CASE("creating cloned strings does not crash") {
    char *abcdef = strdup("abcdef");
    TCString *s = tc_string_clone(abcdef);
    free(abcdef);
    CHECK(strcmp(tc_string_content(s), "abcdef") == 0);
    tc_string_free(s);
}

TEST_CASE("strings echo back their content") {
    TCString *s = tc_string_new("abcdef");
    CHECK(strcmp(tc_string_content(s), "abcdef") == 0);
    tc_string_free(s);
}

TEST_CASE("tc_string_content returns NULL for strings containing embedded NULs") {
    TCString *s = tc_string_clone_with_len("ab\0de", 5);
    REQUIRE(s != NULL);
    CHECK(tc_string_content(s) == NULL);
    tc_string_free(s);
}
