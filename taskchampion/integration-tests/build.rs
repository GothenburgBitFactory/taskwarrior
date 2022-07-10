use std::env;
use std::fs;
use std::path::Path;

/// Build the Unity-based C test suite in `src/bindings_tests`, linking the result with this
/// package's library crate.
fn build_bindings_tests(suites: &[&'static str]) {
    let mut build = cc::Build::new();
    build.include("../lib"); // include path for taskchampion.h
    build.include("src/bindings_tests/unity");
    build.define("UNITY_OUTPUT_CHAR", "test_output");
    build.define(
        "UNITY_OUTPUT_CHAR_HEADER_DECLARATION",
        "test_output(char c)",
    );
    build.file("src/bindings_tests/unity/unity.c");

    let mut files = vec!["src/bindings_tests/test.c".to_string()];
    for suite in suites {
        files.push(format!("src/bindings_tests/{}.c", suite));
    }
    for file in files {
        build.file(&file);
        println!("cargo:rerun-if-changed={}", file);
    }

    build.compile("bindings-tests");
}

/// Make `bindings_test_suites.rs` listing all of the test suites, for use in building the
/// bindings-test binary.
fn make_suite_file(suites: &[&'static str]) {
    let out_dir = env::var_os("OUT_DIR").unwrap();
    let dest_path = Path::new(&out_dir).join("bindings_test_suites.rs");
    let mut content = String::new();
    for suite in suites {
        content.push_str(format!("suite!({}_tests);\n", suite).as_ref());
    }
    fs::write(&dest_path, content).unwrap();
}

fn main() {
    println!("cargo:rerun-if-changed=build.rs");

    let suites = &["uuid", "string", "task", "replica"];
    build_bindings_tests(suites);
    make_suite_file(suites);
}
