use cbindgen::*;

use std::env;

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();

    Builder::new()
        .with_crate(crate_dir)
        .with_language(Language::C)
        .with_config(Config {
            cpp_compat: true,
            ..Default::default()
        })
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file("taskchampion.h");
}
