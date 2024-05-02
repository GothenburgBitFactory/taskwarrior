//! This executable defines the `cargo xtask` subcommands.
//!
//! At the moment it is very simple, but if this grows more subcommands then
//! it will be sensible to use `clap` or another similar library.

use std::env;
use std::fs::File;
use std::io::Write;
use std::path::{Path, PathBuf};

pub fn main() -> anyhow::Result<()> {
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR")?);
    let workspace_dir = manifest_dir.parent().unwrap();
    let arguments: Vec<String> = env::args().collect();

    if arguments.len() < 2 {
        anyhow::bail!("xtask: Valid arguments are: `codegen`");
    }

    match arguments[1].as_str() {
        "codegen" => codegen(workspace_dir),
        _ => anyhow::bail!("xtask: unknown xtask"),
    }
}

/// `cargo xtask codegen`
///
/// This uses ffizz-header to generate `lib/taskchampion.h`.
fn codegen(workspace_dir: &Path) -> anyhow::Result<()> {
    let lib_crate_dir = workspace_dir.join("src/tc/lib");
    let mut file = File::create(lib_crate_dir.join("taskchampion.h")).unwrap();
    write!(&mut file, "{}", ::taskchampion_lib::generate_header()).unwrap();

    Ok(())
}
