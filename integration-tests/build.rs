use std::env;
use std::fs;
use std::path::Path;

fn build_libtaskchampion(suites: &[&'static str]) {
    // This crate has taskchampion-lib in its build-dependencies, so
    // libtaskchampion.so should be built already.  Hopefully it's in target/$PROFILE, and hopefully
    // it's named libtaskchampion.so and not something else
    let mut libtaskchampion = env::current_dir().unwrap();
    libtaskchampion.pop();
    libtaskchampion.push("target");
    libtaskchampion.push(env::var("PROFILE").unwrap());
    libtaskchampion.push("deps");
    libtaskchampion.push("libtaskchampion.so");

    let mut build = cc::Build::new();
    build.object(libtaskchampion);
    build.include("../lib");
    build.include("src/bindings_tests/unity");
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
    build_libtaskchampion(suites);
    make_suite_file(suites);
}
