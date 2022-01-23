use cbindgen::*;

use std::env;

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();

    Builder::new()
        .with_crate(crate_dir)
        .with_language(Language::C)
        .with_config(Config {
            cpp_compat: true,
            export: ExportConfig {
                item_types: vec![
                    ItemType::Structs,
                    ItemType::Globals,
                    ItemType::Functions,
                    ItemType::Constants,
                    ItemType::OpaqueItems,
                ],
                ..Default::default()
            },
            ..Default::default()
        })
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file("taskchampion.h");
}
