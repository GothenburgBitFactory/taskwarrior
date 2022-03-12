//! This executable defines the `cargo xtask` subcommands.
//!
//! At the moment it is very simple, but if this grows more subcommands then
//! it will be sensible to use `clap` or another similar library.

use cbindgen::*;
use std::env;
use std::path::PathBuf;

pub fn main() -> anyhow::Result<()> {
    let arg = env::args().nth(1);
    match arg.as_deref() {
        Some("codegen") => codegen(),
        Some(arg) => anyhow::bail!("unknown xtask {}", arg),
        _ => anyhow::bail!("unknown xtask"),
    }
}

/// `cargo xtask codegen`
///
/// This uses cbindgen to generate `lib/taskchampion.h`.
fn codegen() -> anyhow::Result<()> {
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let workspace_dir = manifest_dir.parent().unwrap();
    let lib_crate_dir = workspace_dir.join("lib");

    Builder::new()
        .with_crate(&lib_crate_dir)
        .with_config(Config {
            header: Some(include_str!("../../lib/header-intro.h").into()),
            language: Language::C,
            include_guard: Some("TASKCHAMPION_H".into()),
            cpp_compat: true,
            sys_includes: vec!["stdbool.h".into(), "stdint.h".into(), "time.h".into()],
            usize_is_size_t: true,
            no_includes: true,
            enumeration: EnumConfig {
                // this appears to still default to true for C
                enum_class: false,
                ..Default::default()
            },
            ..Default::default()
        })
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file(lib_crate_dir.join("taskchampion.h"));

    Ok(())
}
