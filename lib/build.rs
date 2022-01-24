use cbindgen::*;

use std::env;

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();

    Builder::new()
        .with_crate(crate_dir)
        .with_language(Language::C)
        .with_config(Config {
            cpp_compat: true,
            usize_is_size_t: true,
            enumeration: EnumConfig {
                // this appears to still default to true for C
                enum_class: false,
                ..Default::default()
            },
            ..Default::default()
        })
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file("taskchampion.h");
}
