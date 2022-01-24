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
    REQUIRE(s != NULL);
    free(abcdef);

    CHECK(strcmp(tc_string_content(s), "abcdef") == 0);
    tc_string_free(s);
}

TEST_CASE("borrowed strings echo back their content") {
    TCString *s = tc_string_new("abcdef");
    REQUIRE(s != NULL);

    CHECK(strcmp(tc_string_content(s), "abcdef") == 0);
    size_t len;
    const char *buf = tc_string_content_with_len(s, &len);
    REQUIRE(buf != NULL);
    CHECK(len == 6);
    CHECK(strncmp(buf, "abcdef", len) == 0);
    tc_string_free(s);
}

TEST_CASE("cloned strings echo back their content") {
    char *orig = strdup("abcdef");
    TCString *s = tc_string_clone(orig);
    REQUIRE(s != NULL);
    free(orig);

    CHECK(strcmp(tc_string_content(s), "abcdef") == 0);

    size_t len;
    const char *buf = tc_string_content_with_len(s, &len);
    REQUIRE(buf != NULL);
    CHECK(len == 6);
    CHECK(strncmp(buf, "abcdef", len) == 0);
    tc_string_free(s);
}

TEST_CASE("tc_string_content returns NULL for strings containing embedded NULs") {
    TCString *s = tc_string_clone_with_len("ab\0de", 5);
    REQUIRE(s != NULL);

    CHECK(tc_string_content(s) == NULL);

    size_t len;
    const char *buf = tc_string_content_with_len(s, &len);
    REQUIRE(buf != NULL);
    CHECK(len == 5);
    CHECK(strncmp(buf, "ab\0de", len) == 0);
    tc_string_free(s);
}
