//! This executable defines the `cargo xtask` subcommands.
//!
//! At the moment it is very simple, but if this grows more subcommands then
//! it will be sensible to use `clap` or another similar library.

use std::env;
use std::fs::File;
use std::io::Write;
use std::path::PathBuf;

pub fn main() -> anyhow::Result<()> {
    let arg = env::args().nth(1);
    match arg.as_deref() {
        Some("codegen") => codegen(),
        Some("msrv") => msrv(&env::args().nth(2).unwrap(), &env::args().nth(3).unwrap()),
        Some(arg) => anyhow::bail!("unknown xtask {}", arg),
        _ => anyhow::bail!("unknown xtask"),
    }
}

/// `cargo xtask codegen`
///
/// This uses ffizz-header to generate `lib/taskchampion.h`.
fn codegen() -> anyhow::Result<()> {
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let workspace_dir = manifest_dir.parent().unwrap();
    let lib_crate_dir = workspace_dir.join("lib");
    let mut file = File::create(lib_crate_dir.join("taskchampion.h")).unwrap();
    write!(&mut file, "{}", ::taskchampion_lib::generate_header()).unwrap();

    Ok(())
}

/// `cargo xtask msrv (X.Y)`
/// 
/// This updates all of the places in the repo where the MSRV occurs to (X.Y)
/// Currently a placeholder with 'hello world'
fn msrv(x: &String, y: &String) -> anyhow::Result<()> {
    println!("Hello World");
    println!("Parameters are X: {}, Y: {}", x, y);

    Ok(())
}