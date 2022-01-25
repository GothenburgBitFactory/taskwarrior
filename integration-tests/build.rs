fn main() {
    // This crate has taskchampion-lib in its build-dependencies, so
    // libtaskchampion.so should be built already.  Hopefully it's in target/$PROFILE, and hopefully
    // it's named libtaskchampion.so and not something else
    let mut libtaskchampion = std::env::current_dir().unwrap();
    libtaskchampion.pop();
    libtaskchampion.push("target");
    libtaskchampion.push(std::env::var("PROFILE").unwrap());
    libtaskchampion.push("deps");
    libtaskchampion.push("libtaskchampion.so");

    println!("cargo:rerun-if-changed=build.rs");

    let mut build = cc::Build::new();
    build.object(libtaskchampion);
    build.include("../lib");
    build.include("src/bindings_tests/unity");
    build.file("src/bindings_tests/unity/unity.c");

    let files = &[
        "src/bindings_tests/test.c",
        // keep this list in sync with integration-tests/src/bindings_tests/mod.rs and
        // integration-tests/tests/bindings.rs
        "src/bindings_tests/uuid.c",
        "src/bindings_tests/string.c",
        "src/bindings_tests/task.c",
        "src/bindings_tests/replica.c",
    ];

    for file in files {
        build.file(file);
        println!("cargo:rerun-if-changed={}", file);
    }

    build.compile("bindings-tests");
}
