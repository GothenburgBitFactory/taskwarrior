//! This executable defines the `cargo xtask` subcommands.
//!
//! At the moment it is very simple, but if this grows more subcommands then
//! it will be sensible to use `clap` or another similar library.

use std::env;
use std::fs::File;
use std::io::Write;
use std::path::PathBuf;

pub fn main() -> anyhow::Result<()> {
    let arguments: Vec<String> = env::args().collect();
    if arguments.len() < 2 {
        println!("Valid arguments are `codegen`, `msrv <arg1> <arg2>`");
        return Ok(());
    }

    match arguments[1].as_str() {
        "codegen" => codegen(),
        "msrv" => msrv(arguments),
        arg => anyhow::bail!("unknown xtask {}", arg),
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
/// This updates all of the places in the repo where the MSRV occurs to (X.Y) (Currently a Placeholder).
fn msrv(args: Vec<String>) -> anyhow::Result<()> {
    println!("Hello World");

    if args.len() > 3 {
        println!("Arguments are X: {}, Y: {}", args[2], args[3]);
    } else {
        println!("Command `msrv` expects two arguments");
    }

    Ok(())
}
