#include <stdio.h>
#include "unity.h"

// these functions are shared between all test "suites"
// and cannot be customized per-suite.
void setUp(void) { }
void tearDown(void) { }

static FILE *output = NULL;

// Set up for test_output, writing output to "TEST-OUTPUT" in the
// current directory.  The Rust test harness reads this file to get
// the output and display it only on failure.  This is called by
// the Rust test harness
void setup_output(void) {
    output = fopen("TEST-OUTPUT", "w");
}

// Close the output file.  Called by the Rust test harness.
void finish_output(void) {
    fclose(output);
    output = NULL;
}

// this replaces UNITY_OUTPUT_CHAR, and writes output to
// TEST-OUTPUT in the current directory; the Rust test harness
// will read this data if the test fails.
void test_output(char c) {
    fputc(c, output);
}
